#include "indcpa.h"
#include "cbd.h"
#include "poly.h"
#include "randombytes.h"
#include "poly_mul.h"
#include <stdio.h>

/*************************************************
 * Name:        genRx_vec
 *
 * Description: Deterministically generate a vector of sparse polynomial r(x)
 *              from a seed.
 *
 * Arguments:   - polyvec *r: pointer to ouptput vector r
 *              - uint8_t *input: pointer to input seed (of length input_size)
 **************************************************/
void genRx_vec(polyvec* r, const uint8_t* input) {
	unsigned int i;
	uint8_t buf[CBDSEED_BYTES] = { 0 };

	for (i = 0; i < MODULE_RANK; ++i) {
		uint8_t extseed[DELTA_BYTES + 1];
		memcpy(extseed, input, DELTA_BYTES);
		extseed[DELTA_BYTES] = i;

		shake256(buf, CBDSEED_BYTES, extseed, DELTA_BYTES + 1);

		poly_cbd(&r->vec[i], buf);
	}
}

void genRx_poly(poly* r, const uint8_t* input, const unsigned int i) {
	uint8_t buf[CBDSEED_BYTES] = { 0 };
	uint8_t extseed[DELTA_BYTES + 1];
	memcpy(extseed, input, DELTA_BYTES);
	extseed[DELTA_BYTES] = i;

	shake256(buf, CBDSEED_BYTES, extseed, DELTA_BYTES + 1);
	poly_cbd(r, buf);
}

/*************************************************
 * Name:        indcpa_keypair
 *
 * Description: Generates public and private key for the CPA-secure
 *              Module-Lizard public-key encryption scheme.
 *
 * Arguments:   - public_key *pk: pointer to output public key
 *                (a structure composed of (seed of A, matrix A, vector b))
 *              - secret_key *sk: pointer to output private key
 *                (a structure composed of (vector s, t, vector negstart))
 **************************************************/
void indcpa_keypair(uint8_t pk[PUBLICKEY_BYTES], uint8_t sk[PKE_SECRETKEY_BYTES]) {

	//secret_key sk_tmp; //1024byte
	//memset(&sk_tmp, 0, sizeof(secret_key));

	uint8_t seed[CRYPTO_BYTES + PKSEED_BYTES] = { 0 }; //32 + 32
	uint8_t seed_pk[PKSEED_BYTES] = { 0 };

	randombytes(seed, CRYPTO_BYTES);

	sha3_512(seed, seed, CRYPTO_BYTES);

	//genSx_vec(&sk_tmp, seed);
	
	polyvec pkb;
	memset(&pkb, 0, sizeof(pkb));
	
	matacc(&pkb, sk, seed);
	
	memcpy(seed_pk, seed + CRYPTO_BYTES, sizeof(uint8_t) * PKSEED_BYTES);
	memcpy(pk, seed_pk, sizeof(uint8_t) * PKSEED_BYTES);
	
	//save_to_string_sk(sk, &sk_tmp);
	Rq_vec_to_bytes(pk + PKSEED_BYTES, &pkb);
}

/*************************************************
 * Name:        indcpa_enc
 *
 * Description: Encryption function of the CPA-secure
 *              Module-Lizard public-key encryption scheme.
 *
 * Arguments:   - ciphertext *ctxt: pointer to output ciphertext
 *                (a structure composed of (vector c1, c2))
 *              - public_key *pk: pointer to input public key
 *                (a structure composed of (seed of A, matrix A, vector b))
 *              - uint8_t *delta: pointer to input random delta (of length
 *                DELTA_BYTES) to deterministically generate all randomness
 **************************************************/
void indcpa_enc(uint8_t ctxt[CIPHERTEXT_BYTES],
	const uint8_t pk[PUBLICKEY_BYTES],
	const uint8_t mu[MSG_BYTES],
	const uint8_t seed[DELTA_BYTES]) {

	int i;

	uint8_t seed_r[DELTA_BYTES] = { 0 };
	uint8_t seed_pk[PKSEED_BYTES] = { 0 };
	memcpy(seed_pk, pk, PKSEED_BYTES);

	if (seed == NULL)
		randombytes(seed_r, DELTA_BYTES);
	else
		cmov(seed_r, seed, DELTA_BYTES, 1);

	ciphertext ctxt_tmp;
	memset(&ctxt_tmp, 0, sizeof(ciphertext));
	
	//그래서 c2 먼저 생성하도록 해본다
	matacc_c2(&(ctxt_tmp.c2), mu, pk + PKSEED_BYTES, seed_r);

	//C1에서 오버플로우 일어나서 pk 를 덮음 A와 R을 생성할때 오버플로우 나는듯함
	for (i = 0; i < MODULE_RANK; i++) {
		matacc_c1(&ctxt_tmp.c1, seed_r, i, seed_pk);
	}

	for (i = 0; i < MODULE_RANK; ++i) {
		for (int j = 0; j < LWE_N; ++j) {
			ctxt_tmp.c1.vec[i].coeffs[j] = ((ctxt_tmp.c1.vec[i].coeffs[j] + RD_ADD) & RD_AND) >> _16_LOG_P;
		}
	}
	
	save_to_string(ctxt, &ctxt_tmp);
}


/*************************************************
 * Name:        indcpa_dec
 *
 * Description: Decryption function of the CPA-secure
 *              Module-Lizard public-key encryption scheme.
 *
 * Arguments:   - uint8_t *delta: pointer to output decrypted delta
 *                (of length DELTA_BYTES), assumed to be already initialized
 *              - secret_key *sk: pointer to input private key
 *                (a structure composed of (vector s, t, vector negstart)
 *              - ciphertext *ctxt: pointer to input ciphertext
 *                (a structure composed of (vector c1, c2))
 **************************************************/
void indcpa_dec(uint8_t delta[MSG_BYTES],
	const uint8_t sk[PKE_SECRETKEY_BYTES],
	const uint8_t ctxt[CIPHERTEXT_BYTES]) {
		
	//cipher_text 배열을 행렬로 불러오기
	ciphertext ctxt_tmp;						//1.5kb

	load_from_string(&ctxt_tmp, ctxt);
	
	matacc_dec(&ctxt_tmp.c1, sk, &ctxt_tmp.c2);

	//cipher_text 복호화한 값 평문으로 조정
	poly_tomsg(delta, &ctxt_tmp.c2); // EDIT TiMER
}
