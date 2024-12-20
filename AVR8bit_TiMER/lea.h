#ifndef _LEA_H_
#define _LEA_H_

#include <stdint.h>

#define ROR(W,i) (((W)>>(i)) | ((W)<<(32-(i))))
#define ROL(W,i) (((W)<<(i)) | ((W)>>(32-(i))))

#define DWORD_in(x)            (*(uint32_t*)(x))
#define DWORD_out(x, v)        {*((uint32_t*)(x)) = (v);}

void copy_b_array(uint8_t *src, unsigned int len, uint8_t *dest);
void xor_b_array(uint8_t *data, unsigned int len, uint8_t *xor_arr);
void xor_b_array_ctr(uint8_t *data, unsigned int len, uint8_t *xor_arr1, uint8_t *xor_arr2);
void byte2state(uint32_t* state, uint8_t* b);
void state2byte(uint32_t state[4], uint8_t b[16]);
void LEA32_Encrypt(uint8_t* pt, uint32_t rk[24][6], uint8_t* ct);
void LEA32_Decrypt(uint8_t* ct, uint32_t rk[24][6], uint8_t* pt);
void LEA32_Enc_KeySchedule(uint8_t firkey[16], uint32_t rk[24][6]);

#endif
