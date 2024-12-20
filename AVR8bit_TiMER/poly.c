#include "poly.h"
#include <string.h>

/*************************************************
 * Name:        poly_frommsg
 *
 * Description: Convert 16-byte message to polynomial
 *
 * Arguments:   - poly *r:                  pointer to output polynomial
 *              - const unsigned char *msg: pointer to input message
 **************************************************/
void poly_frommsg(poly *r, const uint8_t *msg) {
    unsigned int mask;
    for (size_t i = 0; i < MSG_BYTES; i++) {
        for (size_t j = 0; j < 8; j++) {
            mask = (msg[i] >> j) & 1;
            mask = (mask * Modulus_Q_2) & Modulus_Q_2;
            r->coeffs[8 * i + j] = mask;
            r->coeffs[8 * i + j + 128] = mask;
        }
    }
}

/*************************************************
 * Name:        poly_tomsg
 *
 * Description: Convert polynomial to 16-byte message
 *
 * Arguments:   - unsigned char *msg: pointer to output message
 *              - const poly *x:      pointer to input polynomial
 **************************************************/
void poly_tomsg(unsigned char *msg, const poly *x) {
    uint16_t t;
    for (size_t i = 0; i < MSG_BYTES; i++)
        msg[i] = 0;

    for (size_t i = 0; i < LWE_N / 2; i++) {
        t = flipabs(x->coeffs[i]);
        t += flipabs(x->coeffs[i + 128]);
        t = t - Modulus_Q_2;
        t >>= 15;
        msg[i >> 3] |= t << (i & 7);
    }
}

/*************************************************
 * Name:        flipabs
 *
 * Description: Computes |(x mod q) - Q/2|
 *
 * Arguments:   uint16_t x: input coefficient
 *
 * Returns |(x mod q) - Q/2|
 **************************************************/
uint16_t flipabs(uint16_t x) {
    int16_t r, m;
    r = x - (Modulus_Q_2);
    m = r >> 15;
    return (r + m) ^ m;
}

/*************************************************
 * Name:        poly_add
 *
 * Description: Add two polynomials; no modular reduction is performed
 *
 * Arguments: - poly *r: pointer to output polynomial
 *            - poly *a: pointer to first input polynomial
 *            - poly *b: pointer to second input polynomial
 **************************************************/
void poly_add(poly *r, const poly *a, const poly *b) {
    unsigned int i;
    for (i = 0; i < LWE_N; i++)
        r->coeffs[i] = a->coeffs[i] + b->coeffs[i];
}

/*************************************************
 * Name:        poly_sub
 *
 * Description: Subtract two polynomials; no modular reduction is performed
 *
 * Arguments: - poly *r: pointer to output polynomial
 *            - poly *a: pointer to first input polynomial
 *            - poly *b: pointer to second input polynomial
 **************************************************/
void poly_sub(poly *r, const poly *a, const poly *b) {
    unsigned int i;
    for (i = 0; i < LWE_N; i++)
        r->coeffs[i] = a->coeffs[i] - b->coeffs[i];
}

