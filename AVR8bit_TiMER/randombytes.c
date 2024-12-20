#include "randombytes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// temporary rng
static int i = 0;

static int randombytes_avr_randombytes(uint8_t *buf, size_t n) {
	i++;
	for (int cnt_i = 0; cnt_i < n; cnt_i++)
	{
		buf[cnt_i] = cnt_i + i;
	}
	return 0;
}

int randombytes(uint8_t *output, size_t n) {
	void *buf = (void *)output;
	return randombytes_avr_randombytes(buf, n);
}