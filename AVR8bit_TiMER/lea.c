#include "lea.h"


void copy_b_array(uint8_t *dest, unsigned int len, uint8_t *src) {
	for (unsigned int i = 0; i < len; i++) {
		dest[i] = src[i];
	}
}

void xor_b_array_ctr(uint8_t *data, unsigned int len, uint8_t *xor_arr1, uint8_t *xor_arr2) {
	for (unsigned int i = 0; i < len; i++) {
		data[i] = xor_arr1[i] ^ xor_arr2[i];
	}
}

void xor_b_array(uint8_t *data, unsigned int len, uint8_t *xor_arr) {
	for (unsigned int i = 0; i < len; i++) {
		data[i] ^= xor_arr[i];
	}
}

void LEA32_Enc_KeySchedule(uint8_t firkey[16], uint32_t rk[24][6]) {
	uint32_t delta[4] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec };
	uint32_t T[4] = { 0x0, };
	int i;

	T[0] = DWORD_in(firkey);
	T[1] = DWORD_in(firkey + 4);
	T[2] = DWORD_in(firkey + 8);
	T[3] = DWORD_in(firkey + 12);

	for (i = 0; i < 24; i++)
	{
		T[0] = ROL(T[0] + ROL(delta[i & 3], i), 1);
		T[1] = ROL(T[1] + ROL(delta[i & 3], i + 1), 3);
		T[2] = ROL(T[2] + ROL(delta[i & 3], i + 2), 6);
		T[3] = ROL(T[3] + ROL(delta[i & 3], i + 3), 11);

		rk[i][0] = T[0];
		rk[i][1] = T[1];
		rk[i][2] = T[2];
		rk[i][3] = T[1];
		rk[i][4] = T[3];
		rk[i][5] = T[1];
	}
}

void LEA32_Encrypt(uint8_t* pt, uint32_t rk[24][6], uint8_t* ct) {
	uint32_t X0, X1, X2, X3;
	uint32_t temp;
	int i;

	X0 = DWORD_in(pt);
	X1 = DWORD_in(pt + 4);
	X2 = DWORD_in(pt + 8);
	X3 = DWORD_in(pt + 12);

	for (i = 0; i < 24; i++)
	{
		X3 = ROR((X2 ^ rk[i][4]) + (X3 ^ rk[i][5]), 3);
		X2 = ROR((X1 ^ rk[i][2]) + (X2 ^ rk[i][3]), 5);
		X1 = ROL((X0 ^ rk[i][0]) + (X1 ^ rk[i][1]), 9);
		temp = X0;
		X0 = X1; X1 = X2; X2 = X3; X3 = temp;
	}

	DWORD_out(ct, X0);
	DWORD_out(ct + 4, X1);
	DWORD_out(ct + 8, X2);
	DWORD_out(ct + 12, X3);
}

void LEA32_Decrypt(uint8_t* ct, uint32_t rk[24][6], uint8_t* pt) {
	uint32_t X0, X1, X2, X3;
	uint32_t temp;
	int i;

	X0 = DWORD_in(ct);
	X1 = DWORD_in(ct + 4);
	X2 = DWORD_in(ct + 8);
	X3 = DWORD_in(ct + 12);

	for (i = 0; i < 24; i++)
	{
		temp = X3;
		X3 = X2;
		X2 = X1;
		X1 = X0;
		X0 = temp;

		X1 = (ROR(X1, 9) - (X0 ^ rk[24 - 1 - i][0])) ^ rk[24 - 1 - i][1];
		X2 = (ROL(X2, 5) - (X1 ^ rk[24 - 1 - i][2])) ^ rk[24 - 1 - i][3];
		X3 = (ROL(X3, 3) - (X2 ^ rk[24 - 1 - i][4])) ^ rk[24 - 1 - i][5];
	}

	DWORD_out(pt, X0);
	DWORD_out(pt + 4, X1);
	DWORD_out(pt + 8, X2);
	DWORD_out(pt + 12, X3);
}

