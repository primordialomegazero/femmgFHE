#ifndef CRYPTO_BACKEND_H
#define CRYPTO_BACKEND_H
#include <stddef.h>
typedef struct {
    int (*sha256)(const unsigned char *in, size_t len, unsigned char *out);
    int (*rand_bytes)(unsigned char *buf, size_t len);
} CryptoBackend;
extern CryptoBackend phi_backend;
void crypto_set_backend(CryptoBackend *backend);
CryptoBackend *crypto_get_backend(void);
#endif
