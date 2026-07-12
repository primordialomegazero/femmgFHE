#ifndef PHI_KEM_H
#define PHI_KEM_H

#include <stdint.h>
#include <stddef.h>

#define PHI_KEM_PUBLICKEYBYTES 64
#define PHI_KEM_SECRETKEYBYTES 32
#define PHI_KEM_CIPHERTEXTBYTES 128
#define PHI_KEM_SHAREDSECRETBYTES 32

int phi_kem_keygen(uint8_t *pk, uint8_t *sk);
int phi_kem_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk);
int phi_kem_decaps(uint8_t *ss, const uint8_t *ct, size_t ct_len, const uint8_t *sk);

#endif
