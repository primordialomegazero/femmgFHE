#ifndef SHRINCS_H
#define SHRINCS_H

#include <stddef.h>
#include <stdint.h>

// SHRINCS: Sign, Hash, Ring, Invoice, Nonce, Commitment, Script
// Bitcoin-native cryptographic primitives

#define SHRINCS_PUBLICKEYBYTES 33
#define SHRINCS_SECRETKEYBYTES 32
#define SHRINCS_SIGNATUREBYTES 65
#define SHRINCS_HASHBYTES 32

// Schnorr-based Ring Signature
int shrincs_ring_sign(unsigned char *sig, size_t *siglen,
                       const unsigned char *msg, size_t msglen,
                       const unsigned char **pks, const unsigned char *sk,
                       size_t ring_size);

int shrincs_ring_verify(const unsigned char *sig, size_t siglen,
                         const unsigned char *msg, size_t msglen,
                         const unsigned char **pks, size_t ring_size);

// Nonce commitment scheme
int shrincs_nonce_commit(unsigned char *commitment,
                          const unsigned char *nonce, size_t nonce_len);

// Invoice (BIP-21 style with Schnorr)
int shrincs_invoice_create(char *invoice, size_t invoice_len,
                            const unsigned char *address,
                            uint64_t amount_satoshis,
                            const unsigned char *memo, size_t memo_len);

// Bitcoin Script with Schnorr (Taproot-style)
int shrincs_script_schnorr(unsigned char *script, size_t *script_len,
                            const unsigned char *pk);

#endif
