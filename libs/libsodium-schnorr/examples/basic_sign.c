#include <stdio.h>
#include <string.h>
#include "schnorr.h"

int main() {
    unsigned char pk[SCHNORR_PUBLICKEYBYTES];
    unsigned char sk[SCHNORR_SECRETKEYBYTES];
    unsigned char sig[SCHNORR_BYTES];
    size_t sig_len = sizeof(sig);

    if (schnorr_keypair(pk, sk) != 0) {
        fprintf(stderr, "Keygen failed\n");
        return 1;
    }

    const char *msg = "Hello, Schnorr!";
    if (schnorr_sign((unsigned char*)msg, strlen(msg), sk, sig, &sig_len) != 0) {
        fprintf(stderr, "Sign failed\n");
        return 1;
    }

    if (schnorr_verify(sig, sig_len, (unsigned char*)msg, strlen(msg), pk) == 0) {
        printf("✅ Signature verified!\n");
    }

    sig[0] ^= 0xFF;
    if (schnorr_verify(sig, sig_len, (unsigned char*)msg, strlen(msg), pk) != 0) {
        printf("✅ Tampered signature rejected!\n");
    }

    return 0;
}
