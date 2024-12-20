#include "gcm.h"


void RSHIFT8x16_1(uint8_t v[16]) {
    for (int i = 15; i > 0; i--) {
        v[i] = (v[i] >> 1) | (v[i - 1] << 7);
    }
    v[0] >>= 1;
}

void gcm_gfmul(uint8_t* r, uint8_t* x, uint8_t* y) {
    unsigned char Z[16] = { 0 }, V[16];
    int i;

    copy_b_array(V, 16, y);
    
    for (i = 0; i < 128; i++) {
        if ((x[i >> 3] >> (7 - (i & 0x7))) & 1)
            xor_b_array(Z, 16, V);

        if (V[15] & 1) {
            RSHIFT8x16_1(V);
            V[0] ^= 0xe1;
        }
        else {
            RSHIFT8x16_1(V);
        }
    }

    copy_b_array(r, 16, Z);
}


void GHASH(uint8_t* msg, int msg_len, uint8_t H[16], uint8_t* tag) {
    uint8_t out[16] = { 0 };

    copy_b_array(out, 16, tag);

    for (; msg_len >= 16; msg_len -= 16, msg += 16) {
        xor_b_array(out, 16, msg);
        gcm_gfmul(out, out, H);
    }

    if (msg_len > 0) {
        uint8_t padded_msg[16] = { 0 };
        for (int i = 0; i < msg_len; i++) {
            padded_msg[i] = msg[i];
        }
        xor_b_array(out, 16, padded_msg);
        gcm_gfmul(out, out, H);
    }

    copy_b_array(tag, 16, out);
}


void LEA_GCM_ENC(uint8_t *PT, unsigned int pt_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *CT, uint8_t tag[16]) {
    if (pt_len > MAX_MESSAGE_SIZE) {
        return;
    }

    uint32_t rk[24][6];
    uint8_t first_block[16] = { 0 };
    uint8_t last_block[16] = { 0 };
    uint8_t CTR1[16];
    uint8_t Y[16];
    uint8_t H[16];
    uint8_t Zero[16] = { 0 };

    if (A_len > 0) {
        for (unsigned int j = 0; j < A_len && j < 16; j++) {
            first_block[j] = A[j];
        }
    }

    LEA32_Enc_KeySchedule(key, rk);

    LEA32_Encrypt(CTR, rk, Y);

    LEA32_Encrypt(Zero, rk, H);

    copy_b_array(CTR1, 16, CTR);
    counter_inc(CTR1);

    LEA_CTR(PT, pt_len, key, CTR1, CT);

    unsigned int Alen_bits = A_len << 3;
    unsigned int Clen_bits = pt_len << 3;

    last_block[4] = Alen_bits >> 24;
    last_block[5] = Alen_bits >> 16;
    last_block[6] = Alen_bits >> 8;
    last_block[7] = Alen_bits;

    last_block[12] = Clen_bits >> 24;
    last_block[13] = Clen_bits >> 16;
    last_block[14] = Clen_bits >> 8;
    last_block[15] = Clen_bits;

    unsigned int msg_len = 16 + pt_len + 16;
    uint8_t MSG[MAX_MESSAGE_SIZE + 32] = { 0 };

    for (unsigned int i = 0; i < 16; i++) {
        MSG[i] = first_block[i];
    }
    for (unsigned int i = 0; i < pt_len; i++) {
        MSG[16 + i] = CT[i];
    }
    for (unsigned int i = 0; i < 16; i++) {
        MSG[16 + pt_len + i] = last_block[i];
    }

    GHASH(MSG, msg_len, H, tag);

    xor_b_array(tag, 16, Y);
}


void LEA_GCM_DEC(uint8_t *CT, unsigned int ct_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *PT, uint8_t tag[16]) {
    if (ct_len > MAX_MESSAGE_SIZE) {
        return;
    }

    uint32_t rk[24][6];
    uint8_t first_block[16] = { 0 };
    uint8_t last_block[16] = { 0 };
    uint8_t CTR1[16];
    uint8_t Y[16];
    uint8_t H[16];
    uint8_t Zero[16] = { 0 };

    if (A_len > 0) {
        for (unsigned int j = 0; j < A_len && j < 16; j++) {
            first_block[j] = A[j];
        }
    }

    LEA32_Enc_KeySchedule(key, rk);

    LEA32_Encrypt(CTR, rk, Y);

    LEA32_Encrypt(Zero, rk, H);

    copy_b_array(CTR1, 16, CTR);
    counter_inc(CTR1);

    LEA_CTR(CT, ct_len, key, CTR1, PT);

    unsigned int Alen_bits = A_len << 3;
    unsigned int Clen_bits = ct_len << 3;

    last_block[4] = Alen_bits >> 24;
    last_block[5] = Alen_bits >> 16;
    last_block[6] = Alen_bits >> 8;
    last_block[7] = Alen_bits;

    last_block[12] = Clen_bits >> 24;
    last_block[13] = Clen_bits >> 16;
    last_block[14] = Clen_bits >> 8;
    last_block[15] = Clen_bits;

    unsigned int msg_len = 16 + ct_len + 16;
    uint8_t MSG[MAX_MESSAGE_SIZE + 32] = { 0 };

    for (unsigned int i = 0; i < 16; i++) {
        MSG[i] = first_block[i];
    }
    for (unsigned int i = 0; i < ct_len; i++) {
        MSG[16 + i] = CT[i];
    }
    for (unsigned int i = 0; i < 16; i++) {
        MSG[16 + ct_len + i] = last_block[i];
    }

    GHASH(MSG, msg_len, H, tag);

    xor_b_array(tag, 16, Y);
}
