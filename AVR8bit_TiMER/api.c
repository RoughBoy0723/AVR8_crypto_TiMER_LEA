/*
 * api.c
 *
 * Created: 2024-11-10 오후 1:36:31
 *  Author: BoB13
 */ 
#include "api.h"

void TiMER_keypair(uint8_t *pk, uint8_t *sk){
	indcpa_keypair(pk, sk);
}
void TiMER_enc(uint8_t *ctxt, const uint8_t *pk, const uint8_t *msg){
	uint8_t buf[DELTA_BYTES + CRYPTO_BYTES] = {0}; //64byte
	hash_h(buf, pk, PUBLICKEY_BYTES);
	hash_g(buf, DELTA_BYTES + CRYPTO_BYTES, msg, MSG_BYTES, buf, SHA3_256_HashSize);
	indcpa_enc(ctxt, pk, msg, buf);
}

void TiMER_dec(uint8_t *delta, const uint8_t *sk, const uint8_t *ctxt){
	indcpa_dec(delta, sk, ctxt);
}


void LEA_enc(uint8_t *PT, unsigned int pt_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *CT, uint8_t tag[16]){
	LEA_GCM_ENC(PT, pt_len, CTR, key, A, 16, CT, tag);
}
void LEA_dec(uint8_t *CT, unsigned int ct_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *PT, uint8_t tag[16]){
	LEA_GCM_DEC(CT, ct_len, CTR, key, A, A_len, PT,tag);
}