#ifndef MINISIGN_H
#define MINISIGN_H

#include <stddef.h>
#include <stdint.h>

#define MINISIGN_PUBLICKEYBYTES 33
#define MINISIGN_SECRETKEYBYTES 32
#define MINISIGN_SIGNATUREBYTES 65

int minisign_keypair(unsigned char *pk, unsigned char *sk);
int minisign_sign(const unsigned char *msg, size_t msg_len,
                   const unsigned char *sk,
                   unsigned char *sig, size_t *sig_len,
                   const char *trusted_comment, const char *untrusted_comment);
int minisign_verify(const unsigned char *msg, size_t msg_len,
                     const unsigned char *sig, size_t sig_len,
                     const unsigned char *pk);
int minisign_verify_full(const unsigned char *msg, size_t msg_len,
                          const unsigned char *sig, size_t sig_len,
                          const unsigned char *pk,
                          const char *trusted_comment, const char *untrusted_comment);

#endif
