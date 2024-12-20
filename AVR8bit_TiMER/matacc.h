#ifndef MATACC_H
#define MATACC_H
#include "poly.h"
#include "hash.h"
#include "indcpa.h"
#include "parameters.h"
#include "randombytes.h"
#include "poly_mul.h"
#include <stddef.h>
#include "dg.h"

void matacc(polyvec *pkb, uint8_t* sk, const uint8_t *seed);
void matacc_c1(polyvec* res, const uint8_t* seed_r, int _i, const uint8_t* seed_pk);
void matacc_c2(poly* c2, const uint8_t mu[MSG_BYTES], const uint8_t* pk, const uint8_t* seed_r);
void matacc_dec(polyvec *c1_temp, const uint8_t *sk ,poly *delta_temp);

#endif