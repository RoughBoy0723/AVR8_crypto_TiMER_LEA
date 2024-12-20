#include "kem.h"

/*************************************************
 * Name:        crypto_kem_keypair
 *
 * Description: Generates public and private key
 *              for CCA-secure Module-Lizard key encapsulation mechanism.
 *
 * Arguments:   - public_key *pk: pointer to output public key
 *                (a structure composed of (seed of A, matrix A, vector b))
 *              - secret_key *sk: pointer to output private key
 *                (a structure composed of (vector s, t, vector negstart))
 **************************************************/
int crypto_kem_keypair(uint8_t *pk, uint8_t *sk) {
    indcpa_keypair(pk, sk); //Å° »ýŒº ÈÄ pk[], sk[] ¹è¿­¿¡ ÀúÀå
    randombytes(sk + PKE_SECRETKEY_BYTES, T_BYTES); //secret_key¿¡ ·£Žý ¹ÙÀÌÆ® 32¹ÙÀÌÆ® Ãß°¡
    for (int i = 0; i < PUBLICKEY_BYTES; i++)
        sk[i + PKE_SECRETKEY_BYTES + T_BYTES] = pk[i]; //sk ¹è¿­¿¡ sk, r_seed, pk °ª ÀúÀå
	return 0;
}

/*************************************************
 * Name:        crypto_kem_enc
 *
 * Description: Generates ciphertext and shared
 *              secret for given public key.
 *
 * Arguments:   - ciphertext *ctxt: pointer to output ciphertext
 *                (a structure composed of (vector c21, c22))
 *              - unsigned char *ss: pointer to output shared secret
 *                (an already allocated array of CRYPTO_BYTES bytes)
 *              - public_key *pk: pointer to output public key
 *                (a structure composed of (seed of A, matrix A, vector b))
 *
 * Returns 0(success) or 1(failure).
 **************************************************/
int crypto_kem_enc(uint8_t *ctxt, uint8_t *ss, const uint8_t *pk, const uint8_t *msg) {
    //uint8_t mu[MSG_BYTES] = {0}; // 16byte
    uint8_t buf[DELTA_BYTES + CRYPTO_BYTES] = {0}; //64byte
	
    //memcpy(mu, msg, MSG_BYTES);
    //randombytes(mu, MSG_BYTES); // EDIT TiMER
	
    hash_h(buf, pk, PUBLICKEY_BYTES);
      
    hash_g(buf, DELTA_BYTES + CRYPTO_BYTES, msg, MSG_BYTES, buf, // EDIT TiMER
           SHA3_256_HashSize);

    memset(ss, 0, CRYPTO_BYTES);
    indcpa_enc(ctxt, pk, msg, buf);
    cmov(ss, buf + DELTA_BYTES, CRYPTO_BYTES, 1); 
	
    return 0;
}

/*************************************************
 * Name:        crypto_kem_dec
 *
 * Description: Generates shared secret for given
 *              ciphertext and private key.
 *
 * Arguments:   - unsigned char *ss: pointer to output shared secret
 *                (an already allocated array of CRYPTO_BYTES bytes)
 *              - secret_key *sk: pointer to input private key
 *                (a structure composed of (vector s, t, vector negstart))
 *              - public_key *pk: pointer to input public key
 *                (a structure composed of (seed of A, matrix A, vector b))
 *              - ciphertext *ctxt: pointer to input ciphertext
 *                (a structure composed of (vector c21, c22))
 *
 * Returns 0(success) or 1(failure).
 * On failure, ss will contain a pseudo-random value.
 **************************************************/
int crypto_kem_dec(uint8_t *ss, const uint8_t *ctxt, const uint8_t *sk, const uint8_t *pk, uint8_t *mu) {
    //uint8_t mu[MSG_BYTES] = {0};
    uint8_t buf[DELTA_BYTES + CRYPTO_BYTES] = {0}; // shared secret and seed
    uint8_t buf_tmp[DELTA_BYTES + CRYPTO_BYTES] = {0};
    uint8_t hash_res[SHA3_256_HashSize] = {0};
    //const uint8_t *pk = sk + PKE_SECRETKEY_BYTES + T_BYTES;

    indcpa_dec(mu, sk, ctxt);
	
    hash_h(hash_res, pk, PUBLICKEY_BYTES);
	
    hash_g(buf, DELTA_BYTES + CRYPTO_BYTES, mu, MSG_BYTES, hash_res,
           SHA3_256_HashSize);

    uint8_t ctxt_temp[CIPHERTEXT_BYTES] = {0};
    indcpa_enc(ctxt_temp, pk, mu, buf);

    int fail = verify(ctxt, ctxt_temp, CIPHERTEXT_BYTES);

    hash_h(hash_res, ctxt, CIPHERTEXT_BYTES);
    hash_g(buf_tmp, DELTA_BYTES + CRYPTO_BYTES,
           sk + 2 * MODULE_RANK + SKPOLYVEC_BYTES, T_BYTES, hash_res,
           SHA3_256_HashSize);

    memset(ss, 0, CRYPTO_BYTES);
    cmov(buf + DELTA_BYTES, buf_tmp + DELTA_BYTES, CRYPTO_BYTES, fail);
    cmov(ss, buf + DELTA_BYTES, CRYPTO_BYTES, 1);
    
	return 0;
}
