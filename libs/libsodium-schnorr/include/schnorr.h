#ifndef SCHNORR_H
#define SCHNORR_H

#include <stddef.h>

// ==========================================
// SCHNORR Σ-PROTOCOL (secp256k1)
// ==========================================

#define SCHNORR_PUBLICKEYBYTES 33
#define SCHNORR_SECRETKEYBYTES 32
#define SCHNORR_BYTES 65

int schnorr_keypair(unsigned char *pk, unsigned char *sk);
int schnorr_sign(const unsigned char *msg, size_t msg_len,
                  const unsigned char *priv_key,
                  unsigned char *sig, size_t *sig_len);
int schnorr_verify(const unsigned char *sig, size_t sig_len,
                    const unsigned char *msg, size_t msg_len,
                    const unsigned char *pub_key);

// ==========================================
// SHRINCS — Bitcoin-Native Primitives
// ==========================================
#include "bitcoin/shrincs.h"

// ==========================================
// FRACTAL MODULES
// ==========================================
#include "fractal/schnorr_fractal.h"
#include "fractal/shrincs_fractal.h"

#endif
