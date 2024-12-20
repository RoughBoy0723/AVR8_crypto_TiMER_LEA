#ifndef TiMER_TOOMCOOK_H
#define TiMER_TOOMCOOK_H

#include "parameters.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void poly_mul(const int16_t* a, const int16_t* b, int16_t* res);
void poly_mul_acc(const int16_t* a, const int16_t* b, int16_t* res);

#endif // TiMER_TOOMCOOK_H