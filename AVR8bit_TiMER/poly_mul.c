#include "poly_mul.h"

uint16_t iTC4_modified_scale[7] =
{
	//1, 20389, -21845, -29127, -21845, 20389, 1
	1, 0x4FA5, 0xAAAB, 0x8E39, 0xAAAB, 0x4FA5, 1
};

static void negacyclic_Karatsuba_striding(uint16_t* des, const uint16_t* src1, const uint16_t* src2) {

	//288 byte
	uint16_t res_p00[16] = { 0 }, res_p01[16] = { 0 }, res_p02[16] = { 0 },
	res_p10[16] = { 0 }, res_p11[16] = { 0 }, res_p12[16] = { 0 },
	res_p20[16] = { 0 }, res_p21[16] = { 0 }, res_p22[16] = { 0 };

	//18byte
	uint16_t p00, p01, p02,
	p10, p11, p12,
	p20, p21, p22;

	//12byte
	uint16_t _p01, _p11, _p21;

	uint16_t buff[3];

	for (size_t i = 0; i < 16; i++) {

		// Load 4, cache 9.
		p00 = src1[4 * i + 0];
		p02 = src1[4 * i + 1];
		p20 = src1[4 * i + 2];
		p22 = src1[4 * i + 3];

		p01 = p00 + p02;
		p21 = p20 + p22;

		p10 = p00 + p20;
		p12 = p02 + p22;

		p11 = p10 + p12;

		for (size_t j = 0; j < 16 - i; j++) {

			res_p00[i + j] += p00 * src2[4 * j + 0];
			res_p02[i + j] += p02 * src2[4 * j + 1];
			_p01 = src2[4 * j + 0] + src2[4 * j + 1];
			res_p01[i + j] += p01 * _p01;

			res_p20[i + j] += p20 * src2[4 * j + 2];
			res_p22[i + j] += p22 * src2[4 * j + 3];
			_p21 = src2[4 * j + 2] + src2[4 * j + 3];
			res_p21[i + j] += p21 * _p21;

			res_p10[i + j] += p10 * (src2[4 * j + 0] + src2[4 * j + 2]);
			res_p12[i + j] += p12 * (src2[4 * j + 1] + src2[4 * j + 3]);
			_p11 = _p01 + _p21;
			res_p11[i + j] += p11 * _p11;

		}
		
		for (size_t j = 16 - i; j < 16; j++) {

			res_p00[i + j - 16] -= p00 * src2[4 * j + 0];
			res_p02[i + j - 16] -= p02 * src2[4 * j + 1];
			_p01 = src2[4 * j + 0] + src2[4 * j + 1];
			res_p01[i + j - 16] -= p01 * _p01;

			res_p20[i + j - 16] -= p20 * src2[4 * j + 2];
			res_p22[i + j - 16] -= p22 * src2[4 * j + 3];
			_p21 = src2[4 * j + 2] + src2[4 * j + 3];
			res_p21[i + j - 16] -= p21 * _p21;

			res_p10[i + j - 16] -= p10 * (src2[4 * j + 0] + src2[4 * j + 2]);
			res_p12[i + j - 16] -= p12 * (src2[4 * j + 1] + src2[4 * j + 3]);
			_p11 = _p01 + _p21;
			res_p11[i + j - 16] -= p11 * _p11;

		}
	}

	// Load 9, store 6.
	for (size_t i = 0; i < 16; i++) {
		res_p01[i] = res_p01[i] - res_p00[i] - res_p02[i];
		res_p11[i] = res_p11[i] - res_p10[i] - res_p12[i];
		res_p21[i] = res_p21[i] - res_p20[i] - res_p22[i];

		res_p10[i] = res_p10[i] - res_p00[i] - res_p20[i];
		res_p11[i] = res_p11[i] - res_p01[i] - res_p21[i];
		res_p12[i] = res_p12[i] - res_p02[i] - res_p22[i];

		res_p10[i] += res_p02[i];
		res_p20[i] += res_p12[i];
	}

	// Load 11, store 4, Cache 3.
	des[0] = res_p00[0] - res_p20[15];
	des[1] = res_p01[0] - res_p21[15];
	des[2] = res_p10[0] - res_p22[15];
	des[3] = res_p11[0];
	buff[0] = res_p20[0];
	buff[1] = res_p21[0];
	buff[2] = res_p22[0];
	// Load 7, store 4, cache 3.
	for (size_t i = 1; i < 15; i++) {
		des[4 * i + 0] = buff[0] + res_p00[i];
		des[4 * i + 1] = buff[1] + res_p01[i];
		des[4 * i + 2] = buff[2] + res_p10[i];
		des[4 * i + 3] = res_p11[i];
		buff[0] = res_p20[i];
		buff[1] = res_p21[i];
		buff[2] = res_p22[i];
	}
	// Load 4, store 4.
	des[60] = buff[0] + res_p00[15];
	des[61] = buff[1] + res_p01[15];
	des[62] = buff[2] + res_p10[15];
	des[63] = res_p11[15];

}

static void TC_striding_mul(uint16_t* des, uint16_t* src1, uint16_t* src2) {

	//2.8kb
	uint16_t src1_extended[7][64], src2_extended[7][64];
	uint16_t res[7][64];
	uint16_t TC4_buff[7];
	// uint16_t twiddle;
	uint16_t t0, t1, t2, t3;

	// Apply Toom-4 evaluation matrix.
	for (size_t i = 0; i < 64; i++) {
		src1_extended[0][i] = src1[i * 4 + 0];
		t0 = src1[i * 4 + 0] + src1[i * 4 + 2];
		t1 = src1[i * 4 + 1] + src1[i * 4 + 3];
		src1_extended[1][i] = t0 + t1;
		src1_extended[2][i] = t0 - t1;
		t2 = src1[i * 4 + 0] + 4 * src1[i * 4 + 2];
		t3 = src1[i * 4 + 1] + 4 * src1[i * 4 + 3];
		src1_extended[3][i] = t2 + 2 * t3;
		src1_extended[4][i] = t2 - 2 * t3;
		src1_extended[5][i] = ((src1[i * 4 + 0] * 2 + src1[i * 4 + 1]) * 2 + src1[i * 4 + 2]) * 2 + src1[i * 4 + 3];
		src1_extended[6][i] = src1[i * 4 + 3];
	}

	for (size_t i = 0; i < 64; i++) {
		src2_extended[0][i] = src2[i * 4 + 0];
		t0 = src2[i * 4 + 0] + src2[i * 4 + 2];
		t1 = src2[i * 4 + 1] + src2[i * 4 + 3];
		src2_extended[1][i] = t0 + t1;
		src2_extended[2][i] = t0 - t1;
		t2 = src2[i * 4 + 0] + 4 * src2[i * 4 + 2];
		t3 = src2[i * 4 + 1] + 4 * src2[i * 4 + 3];
		src2_extended[3][i] = t2 + 2 * t3;
		src2_extended[4][i] = t2 - 2 * t3;
		src2_extended[5][i] = ((src2[i * 4 + 0] * 2 + src2[i * 4 + 1]) * 2 + src2[i * 4 + 2]) * 2 + src2[i * 4 + 3];
		src2_extended[6][i] = src2[i * 4 + 3];
	}

	// Compute small-dimensional products.
	for (size_t i = 0; i < 7; i++) {
		negacyclic_Karatsuba_striding((uint16_t*)&res[i][0], (uint16_t*)&src1_extended[i][0], (uint16_t*)&src2_extended[i][0]);
	}

	// Apply Toom-4 inversion matrix.
	for (size_t i = 0; i < 64; i++) {

		// {-360, -120,  -40,    5,     3,    8, -360},
		t0 = ((res[0][i] + res[6][i]) * 3 + res[1][i]) * 3 + res[2][i];
		t0 = res[5][i] - t0 * 5;
		t0 *= 8;
		t0 += res[3][i] * 5;
		t0 += res[4][i] * 3;
		TC4_buff[1] = t0 * iTC4_modified_scale[1];

		// { -30,   16,   16,   -1,    -1,    0,   96},
		t0 = res[1][i] + res[2][i];
		t1 = res[3][i] + res[4][i];
		t0 = 16 * t0 - t1;
		t0 = t0 - 30 * res[0][i];
		t0 += 96 * res[6][i];
		TC4_buff[2] = t0 * iTC4_modified_scale[2];

		// {  45,   27,   -7,   -1,     0,   -1,   45},
		t0 = res[0][i] + res[6][i];
		t1 = res[3][i] + res[5][i];
		t0 = 45 * t0 - t1;
		t0 += 27 * res[1][i];
		t0 = t0 - 7 * res[2][i];
		TC4_buff[3] = t0 * iTC4_modified_scale[3];

		// {   6,   -4,   -4,    1,     1,    0, -120},
		t0 = res[1][i] + res[2][i];
		t1 = res[3][i] + res[4][i];
		t0 = t1 - 4 * t0;
		t0 += 6 * res[0][i];
		t0 = t0 - 120 * res[6][i];
		TC4_buff[4] = t0 * iTC4_modified_scale[4];

		// { -90,  -60,   20,    5,    -3,    2,  -90},
		t0 = res[0][i] + res[6][i];
		t0 *= -90;
		t0 = t0 - 60 * res[1][i];
		t0 += 20 * res[2][i];
		t0 += 5 * res[3][i];
		t0 = t0 - 3 * res[4][i];
		t0 += 2 * res[5][i];
		TC4_buff[5] = t0 * iTC4_modified_scale[5];

		// Multiply by powers of two.
		res[1][i] = TC4_buff[1] >> 2;
		res[2][i] = TC4_buff[2] >> 3;
		res[3][i] = TC4_buff[3] >> 1;
		res[4][i] = TC4_buff[4] >> 3;
		res[5][i] = TC4_buff[5] >> 2;
	}

	// Export the result.
	for (size_t i = 0; i < 64; i++) {
		des[i * 4 + 3] = res[3][i];
	}

	for (size_t i = 0; i < 63; i++) {
		des[(i + 1) * 4 + 0] = res[0][i + 1] + res[4][i];
		des[(i + 1) * 4 + 1] = res[1][i + 1] + res[5][i];
		des[(i + 1) * 4 + 2] = res[2][i + 1] + res[6][i];
	}

	des[0] = res[0][0] - res[4][63];
	des[1] = res[1][0] - res[5][63];
	des[2] = res[2][0] - res[6][63];
}

void poly_mul(const int16_t* a, const int16_t* b, int16_t* res)
{
	TC_striding_mul((uint16_t*)res, (uint16_t*)a, (uint16_t*)b);
}

void poly_mul_acc(const int16_t* a, const int16_t* b, int16_t* res)
{
	TC_striding_mul((uint16_t*)b, (uint16_t *)a, (uint16_t*)b);
	for (int cnt_i = 0; cnt_i < LWE_N; cnt_i++)
		res[cnt_i] += b[cnt_i];
}