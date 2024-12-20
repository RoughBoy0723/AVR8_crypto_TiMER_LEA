#include "matacc.h"

void matacc(polyvec* pkb, uint8_t* sk, const uint8_t* seed ) {
	unsigned int i, j, k;
	poly A;
	poly S;
	poly res;
	
	uint8_t h_buf[PKPOLY_BYTES] = { 0 }, errseed[CRYPTO_BYTES] = { 0 }, pkseed[PKSEED_BYTES + 2];

	memcpy(errseed, seed, CRYPTO_BYTES);
	memcpy(pkseed, seed + CRYPTO_BYTES, PKSEED_BYTES);

	addGaussianErrorVec(pkb, errseed);

	//genAx, vec_vec_mult, poly_sub
	for (i = 0; i < MODULE_RANK; i++) {
		memset(&res, 0, sizeof(poly));

		//vec_vec_mult
		for (j = 0; j < MODULE_RANK; j++) {
			pkseed[32] = i;
			pkseed[33] = j;

			shake128(h_buf, PKPOLY_BYTES, pkseed, PKSEED_BYTES + 2);

			bytes_to_Rq(&A, h_buf);

			for (k = 0; k < LWE_N; ++k)
				A.coeffs[k] = A.coeffs[k] >> _16_LOG_Q;
	
			
						
			if(i == 0){
				genSx_poly(&S, seed, j);
				Sx_to_bytes(sk + SKPOLY_BYTES * j, &S);
			}else{
				bytes_to_Sx(&S, sk+ j * SKPOLY_BYTES);
			}
			
			//A * s = res
			poly_mul_acc(A.coeffs, S.coeffs, res.coeffs);
		}

		// matrix_vec_mult_sub
		for (j = 0; j < LWE_N; ++j)
			res.coeffs[j] <<= _16_LOG_Q;

		//poly sub
		for (j = 0; j < LWE_N; j++)
			pkb->vec[i].coeffs[j] = pkb->vec[i].coeffs[j] - res.coeffs[j];
	}
}

void matacc_c1(polyvec* res, const uint8_t* seed_r, int _i, const uint8_t* seed_pk) {
	unsigned int k, l, rank;
	poly A;
	poly r;
	uint8_t h_buf[PKPOLY_BYTES] = { 0 };
	uint8_t tmpseed[PKSEED_BYTES + 2];

	memcpy(tmpseed, seed_pk, PKSEED_BYTES);

	for (rank = 0; rank < MODULE_RANK; rank++) {
		tmpseed[32] = rank;
		tmpseed[33] = _i;

		shake128(h_buf, PKPOLY_BYTES, tmpseed, PKSEED_BYTES + 2);

		bytes_to_Rq(&A, h_buf);

		for (k = 0; k < LWE_N; ++k)
			A.coeffs[k] = A.coeffs[k] >> _16_LOG_Q;

		genRx_poly(&r, seed_r, rank);

		poly_mul_acc(A.coeffs, r.coeffs, res->vec[_i].coeffs);
	}
	for (l = 0; l < LWE_N; ++l)
		res->vec[_i].coeffs[l] <<= _16_LOG_Q;
}


void matacc_c2(poly* c2, const uint8_t mu[MSG_BYTES], const uint8_t* pk, const uint8_t* seed_r) {
	poly b, res, r;

	unsigned int i, j;
	memset(&res, 0, sizeof(poly));
	
	poly_frommsg(c2, mu);
	
	for (i = 0; i < MODULE_RANK; i++) {
		
		bytes_to_Rq(&b, pk + i * PKPOLY_BYTES);
		
		for (j = 0; j < LWE_N; ++j)
			b.coeffs[j] = b.coeffs[j] >> _16_LOG_Q;

		genRx_poly(&r, seed_r, i);
	
		poly_mul_acc(b.coeffs, r.coeffs, res.coeffs);
	}

	for (j = 0; j < LWE_N; ++j)
		res.coeffs[j] <<= _16_LOG_Q;

	for (i = 0; i < LWE_N; i++)
		c2->coeffs[i] = c2->coeffs[i] + res.coeffs[i];

	for (i = 0; i < LWE_N; ++i) {
		c2->coeffs[i] = ((c2->coeffs[i] + RD_ADD2) & RD_AND2) >> _16_LOG_P2;
	}

}

void matacc_dec(polyvec* c1_temp, const uint8_t* sk, poly* delta_temp) {
	unsigned int i, j;
	
	poly res;									//0.5kb
	poly poly_sk;

	memset(&res, 0, sizeof(poly));

	for (i = 0; i < LWE_N; ++i)
		delta_temp->coeffs[i] <<= _16_LOG_P2;

	for (i = 0; i < MODULE_RANK; i++) {

		for (j = 0; j < LWE_N; ++j)
			c1_temp->vec[i].coeffs[j] <<= _16_LOG_P;

		for (j = 0; j < LWE_N; ++j) {
			c1_temp->vec[i].coeffs[j] = c1_temp->vec[i].coeffs[j] >> _16_LOG_P;
		}

		memset(&poly_sk, 0, sizeof(poly));
		bytes_to_Sx(&poly_sk, sk + SKPOLY_BYTES * i);
		
		poly_mul_acc(c1_temp->vec[i].coeffs, poly_sk.coeffs, res.coeffs);

	}

	//결과값 범위 조절
	for (j = 0; j < LWE_N; ++j)
		res.coeffs[j] <<= _16_LOG_P;

	// 행렬 덧셈
	for (i = 0; i < LWE_N; i++)
		delta_temp->coeffs[i] = delta_temp->coeffs[i] + res.coeffs[i];
}