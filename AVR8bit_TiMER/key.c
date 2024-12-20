#include "key.h"
#include "poly.h"
#include "randombytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*************************************************
 * Name:        genSx_vec
 *
 * Description: Generate a vector of secret sparse polynomial s(x) from a seed.
 *
 * Arguments:   - secret_key *sk: pointer to output private key
 *              - const uint8_t *seed: pointer to a input seed of s(x) (of
 *                                     length CRYPTO_BYTES)
 **************************************************/
void genSx_vec(secret_key *sk, const uint8_t seed[CRYPTO_BYTES]) {
    unsigned int i, j;
    uint8_t extseed[CRYPTO_BYTES + 2] = {0};
    memcpy(extseed, seed, CRYPTO_BYTES);

    for (i = 0; i < MODULE_RANK; ++i) {
        extseed[CRYPTO_BYTES] = i * MODULE_RANK;
        j = 0;
        do {
            extseed[CRYPTO_BYTES + 1] = j;
            j += 1;
        } while(hwt((uint16_t *)sk->vec[i].coeffs, extseed));
    }
}

void genSx_poly(poly* poly_sk, const uint8_t seed[CRYPTO_BYTES], unsigned int i) {
	unsigned int j = 0;
	uint8_t extseed[CRYPTO_BYTES + 2] = { 0 };
	memcpy(extseed, seed, CRYPTO_BYTES);

	extseed[CRYPTO_BYTES] = i * MODULE_RANK;
	do {
		extseed[CRYPTO_BYTES + 1] = j;
		j += 1;
	} while (hwt((uint16_t*)poly_sk->coeffs, extseed));
}