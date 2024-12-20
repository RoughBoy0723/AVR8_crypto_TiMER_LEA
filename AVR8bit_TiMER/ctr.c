#include "ctr.h"

void counter_inc(uint8_t *counter) {
    unsigned int n = 16;
    uint8_t c;

    do {
        --n;
        c = counter[n];
        ++c;
        counter[n] = c;
        if (c) return;
    } while (n);
}

void LEA_CTR(uint8_t *PT, unsigned int pt_len, uint8_t key[16], uint8_t CTR[16], uint8_t *CT) {
    if (pt_len > MAX_MESSAGE_SIZE) {
        //printf("Error: Message length exceeds the maximum allowed size of %d bytes\n", MAX_MESSAGE_SIZE);
        return;
    }

    unsigned int num_blocks = pt_len / 16;
    unsigned int remainder = pt_len & 15;

    uint8_t ctr_ct[16];
    uint32_t rk[24][6];
    uint8_t current_ctr[16];

    LEA32_Enc_KeySchedule(key, rk);

    copy_b_array(current_ctr, 16, CTR);

    while (num_blocks--) {
        LEA32_Encrypt(current_ctr, rk, ctr_ct);
        xor_b_array_ctr(CT, 16, PT, ctr_ct);
        counter_inc(current_ctr);
        PT += 16;
        CT += 16;
    }

    if (remainder & 0xf) {
        LEA32_Encrypt(current_ctr, rk, ctr_ct);
        xor_b_array_ctr(CT, remainder & 0xf, PT, ctr_ct);
    }
}