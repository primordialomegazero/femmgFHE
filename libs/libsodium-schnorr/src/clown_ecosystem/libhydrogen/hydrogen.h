#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stddef.h>
#include <stdint.h>

// libhydrogen-compatible API using Schnorr
#define hydro_sign_PUBLICKEYBYTES 33
#define hydro_sign_SECRETKEYBYTES 32
#define hydro_sign_BYTES 65

int hydro_sign_keypair(unsigned char *pk, unsigned char *sk);
int hydro_sign_create(unsigned char *sig, size_t *sig_len,
                       const unsigned char *msg, size_t msg_len,
                       const unsigned char *sk);
int hydro_sign_verify(const unsigned char *sig, size_t sig_len,
                       const unsigned char *msg, size_t msg_len,
                       const unsigned char *pk);

// Additional libhydrogen features
int hydro_hash_hash(unsigned char *out, size_t out_len,
                     const unsigned char *in, size_t in_len);
int hydro_secretbox_encrypt(unsigned char *ct, const unsigned char *msg, size_t msg_len,
                             const unsigned char *nonce, const unsigned char *key);
int hydro_secretbox_decrypt(unsigned char *msg, const unsigned char *ct, size_t ct_len,
                             const unsigned char *nonce, const unsigned char *key);

#endif
