#ifndef ED25519_H
#define ED25519_H
#include <stddef.h>
#define ED25519_PUBLICKEYBYTES 32
#define ED25519_SECRETKEYBYTES 32
#define ED25519_BYTES 64
int ed25519_keypair(unsigned char *pk, unsigned char *sk);
int ed25519_sign(unsigned char *sig, size_t *siglen,
                  const unsigned char *msg, size_t msglen,
                  const unsigned char *sk);
int ed25519_verify(const unsigned char *sig, size_t siglen,
                    const unsigned char *msg, size_t msglen,
                    const unsigned char *pk);
#endif
