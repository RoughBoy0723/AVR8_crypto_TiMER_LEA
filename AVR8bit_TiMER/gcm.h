#ifndef _GCM_H_
#define _GCM_H_

#include "parameters.h"
#include "lea.h"
#include "ctr.h"

void LEA_GCM_ENC(uint8_t CT[], unsigned int ct_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t PT[], uint8_t tag[16]);
void LEA_GCM_DEC(uint8_t CT[], unsigned int ct_len, uint8_t CTR[16], uint8_t key[16], uint8_t* A, unsigned int A_len, uint8_t PT[], uint8_t tag[16]);

#endif