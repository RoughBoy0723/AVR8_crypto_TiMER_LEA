#ifndef _CTR_H_
#define _CTR_H_

#include "lea.h"
#include "parameters.h"
#include <stdio.h>
#include <stdint.h>

void counter_inc(uint8_t counter[16]);
void LEA_CTR(uint8_t PT[], unsigned int pt_len, uint8_t key[16], uint8_t CTR[16], uint8_t CT[]);

#endif