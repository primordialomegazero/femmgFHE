#include <stdio.h>
#include <string.h>
#include "schnorr.h"

int main() {
    unsigned char pk[SCHNORR_PUBLICKEYBYTES];
    unsigned char sk[SCHNORR_SECRETKEYBYTES];
    unsigned char sig[SCHNORR_BYTES];
    size_t siglen = sizeof(sig);

    if (schnorr_keypair(pk, sk) != 0) {
        printf("Keygen: FAIL\n");
        return 1;
    }

    const char *msg = "Schnorr test message";
    if (schnorr_sign((unsigned char*)msg, strlen(msg), sk, sig, &siglen) != 0) {
        printf("Sign: FAIL\n");
        return 1;
    }

    if (schnorr_verify(sig, siglen, (unsigned char*)msg, strlen(msg), pk) == 0) {
        printf("Schnorr: PASS\n");
        return 0;
    } else {
        printf("Schnorr: FAIL\n");
        return 1;
    }
}
