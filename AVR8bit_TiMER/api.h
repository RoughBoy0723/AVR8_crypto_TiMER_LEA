#ifndef KEM_TiMER_H
#define KEM_TiMER_H

#include <stdint.h>
#include <stdio.h>
#include "gcm.h"
#include "hash.h"
#include "indcpa.h"
#include "parameters.h"

#define TiMER_SECRETKEYBYTES 160 + 672
#define TiMER_PUBLICKEYBYTES 672
#define TiMER_CIPHERTEXTBYTES 608
#define TiMER_BYTES 16

void TiMER_keypair(uint8_t *pk, uint8_t *sk);
void TiMER_enc(uint8_t *ctxt, const uint8_t *pk, const uint8_t *msg);
void TiMER_dec(uint8_t *delta, const uint8_t *sk, const uint8_t *ctxt);

void LEA_enc(uint8_t *PT, unsigned int pt_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *CT, uint8_t tag[16]);
void LEA_dec(uint8_t *CT, unsigned int ct_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t *PT, uint8_t tag[16]);
#endif // KEM_TiMER_H
