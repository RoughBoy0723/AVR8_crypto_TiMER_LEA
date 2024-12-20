#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "api.h"
#include "parameters.h"

uint8_t pk[PUBLICKEY_BYTES] = {0};        // 672byte seed 32, A0 320, A1 320
uint8_t sk[SECRETKEY_BYTES] = {0};    //cbd 128
uint8_t cipher_key[CIPHERTEXT_BYTES] = {0};	  // 608byte	c1 -> 512, c2 -> 96
uint8_t aad[16] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
uint8_t CTR0[16] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0x00, 0x00, 0x00, 0x01 };
uint8_t cnt = 0;

int main(void) {
	uint8_t tag[16] = { 0 };
	uint8_t dec_tag[16] = { 0 };
		
	uint8_t pt[MAX_MESSAGE_SIZE] = {0};	//16
	uint8_t ct[MAX_MESSAGE_SIZE] = {0};	//16
	uint8_t dec_ct[MAX_MESSAGE_SIZE] = {0};		//168
	uint8_t sym_key[MSG_BYTES] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}; //16
	uint8_t dec_sym_key[MSG_BYTES] = { 0 };	//16
			
	TiMER_keypair(pk, sk);

	TiMER_enc(cipher_key, pk, sym_key);
	
	TiMER_dec(dec_sym_key, sk, cipher_key);
	
	//use sym_key enc
	LEA_enc(pt, MAX_MESSAGE_SIZE, CTR0, dec_sym_key, aad, 16, ct, tag);
	
	
	//use dec_sym_key dec
	LEA_dec(ct, MAX_MESSAGE_SIZE, CTR0, dec_sym_key, aad, 16, dec_ct, dec_tag );
	
	return 0;
}