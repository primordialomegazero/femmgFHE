#include "ed25519.h"
#include <sodium.h>
#include <string.h>

static int sodium_init_done = 0;

static void ensure_sodium() {
    if (!sodium_init_done) {
        if (sodium_init() < 0) return;
        sodium_init_done = 1;
    }
}

int ed25519_keypair(unsigned char *pk, unsigned char *sk) {
    ensure_sodium();
    crypto_sign_ed25519_keypair(pk, sk);
    return 0;
}

int ed25519_sign(unsigned char *sig, size_t *siglen,
                  const unsigned char *msg, size_t msglen,
                  const unsigned char *sk) {
    ensure_sodium();
    unsigned long long siglen_ll;
    crypto_sign_ed25519_detached(sig, &siglen_ll, msg, msglen, sk);
    *siglen = (size_t)siglen_ll;
    return 0;
}

int ed25519_verify(const unsigned char *sig, size_t siglen,
                    const unsigned char *msg, size_t msglen,
                    const unsigned char *pk) {
    ensure_sodium();
    return crypto_sign_ed25519_verify_detached(sig, msg, msglen, pk);
}
