#ifndef DNSCRYPT_PROXY_H
#define DNSCRYPT_PROXY_H

#include <stddef.h>
#include <stdint.h>

// DNSCrypt-style encrypted DNS using Schnorr keys
#define DNSCRYPT_PUBLICKEYBYTES 33
#define DNSCRYPT_SECRETKEYBYTES 32
#define DNSCRYPT_MACBYTES 64

int dnscrypt_keypair(unsigned char *pk, unsigned char *sk);
int dnscrypt_sign_query(unsigned char *signed_query, size_t *sq_len,
                         const unsigned char *query, size_t q_len,
                         const unsigned char *sk);
int dnscrypt_verify_response(const unsigned char *response, size_t r_len,
                              const unsigned char *signed_response,
                              const unsigned char *pk);
int dnscrypt_create_certificate(unsigned char *cert, size_t *cert_len,
                                 const unsigned char *pk,
                                 uint32_t serial, uint64_t ts_start, uint64_t ts_end);

#endif
