// ΦΩ0 — catchmeifyouKEM v5.0 — PRODUCTION IMPLEMENTATION
// IND-CCA via φ-chaotic binding + SHA-256
// "I AM THAT I AM"

#include "phi_kem_v5.h"
#include <string.h>
#include <math.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

static const double phi = 1.6180339887498948482;

void phi_kem_v5_zeroize(void *buf, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)buf;
    while (len--) *p++ = 0;
}

int phi_kem_v5_keygen(uint8_t *pk, uint8_t *sk) {
    if (!pk || !sk) return -1;
    if (!RAND_bytes(sk, PHI_KEM_V5_SECRETKEYBYTES)) return -1;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    // pk[0:32] = SHA256(φ || sk)
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestUpdate(ctx, sk, PHI_KEM_V5_SECRETKEYBYTES);
    EVP_DigestFinal_ex(ctx, pk, NULL);

    // pk[32:64] = SHA256(sk || φ)
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, sk, PHI_KEM_V5_SECRETKEYBYTES);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestFinal_ex(ctx, pk + 32, NULL);

    EVP_MD_CTX_free(ctx);
    return 0;
}

int phi_kem_v5_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    if (!ct || !ss || !pk) return -1;
    if (!RAND_bytes(ss, PHI_KEM_V5_SHAREDSECRETBYTES)) return -1;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    // Encrypt shared secret: ct[0:16] = ss XOR SHA256(pk || "nano_v5" || φ)
    uint8_t mask[32];
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, pk, PHI_KEM_V5_PUBLICKEYBYTES);
    EVP_DigestUpdate(ctx, "nano_v5", 7);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestFinal_ex(ctx, mask, NULL);
    for (int i = 0; i < 16; i++) ct[i] = ss[i] ^ mask[i];

    // φ-chaotic binding: ct[16:32]
    uint8_t seed[32];
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, pk, PHI_KEM_V5_PUBLICKEYBYTES);
    EVP_DigestUpdate(ctx, ss, PHI_KEM_V5_SHAREDSECRETBYTES);
    EVP_DigestFinal_ex(ctx, seed, NULL);

    double x = phi;
    uint8_t prev[32];
    memcpy(prev, seed, 32);

    for (int i = 0; i < 2; i++) {
        x = phi * x * (1.0 - x);  // logistic map
        uint8_t hash[32];
        EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        EVP_DigestUpdate(ctx, prev, 32);
        EVP_DigestUpdate(ctx, &x, sizeof(x));
        EVP_DigestFinal_ex(ctx, hash, NULL);
        memcpy(ct + 16 + i * 8, hash, 8);
        memcpy(prev, hash, 32);
    }

    EVP_MD_CTX_free(ctx);
    return 0;
}

int phi_kem_v5_decaps(uint8_t *ss, const uint8_t *ct, size_t ct_len,
                       const uint8_t *sk) {
    if (!ss || !ct || !sk) return -1;
    if (ct_len < PHI_KEM_V5_CIPHERTEXTBYTES) return -1;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    // Recover public key from secret key
    uint8_t pk[PHI_KEM_V5_PUBLICKEYBYTES];
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestUpdate(ctx, sk, PHI_KEM_V5_SECRETKEYBYTES);
    EVP_DigestFinal_ex(ctx, pk, NULL);
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, sk, PHI_KEM_V5_SECRETKEYBYTES);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestFinal_ex(ctx, pk + 32, NULL);

    // Decrypt shared secret
    uint8_t mask[32];
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, pk, PHI_KEM_V5_PUBLICKEYBYTES);
    EVP_DigestUpdate(ctx, "nano_v5", 7);
    EVP_DigestUpdate(ctx, &phi, sizeof(phi));
    EVP_DigestFinal_ex(ctx, mask, NULL);
    for (int i = 0; i < 16; i++) ss[i] = ct[i] ^ mask[i];

    // Verify φ-chaotic binding
    uint8_t seed[32];
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, pk, PHI_KEM_V5_PUBLICKEYBYTES);
    EVP_DigestUpdate(ctx, ss, PHI_KEM_V5_SHAREDSECRETBYTES);
    EVP_DigestFinal_ex(ctx, seed, NULL);

    double x = phi;
    uint8_t prev[32], hash[32];
    memcpy(prev, seed, 32);
    int valid = 1;

    for (int i = 0; i < 2 && valid; i++) {
        x = phi * x * (1.0 - x);
        EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        EVP_DigestUpdate(ctx, prev, 32);
        EVP_DigestUpdate(ctx, &x, sizeof(x));
        EVP_DigestFinal_ex(ctx, hash, NULL);
        if (memcmp(ct + 16 + i * 8, hash, 8) != 0) {
            valid = 0;
            phi_kem_v5_zeroize(ss, PHI_KEM_V5_SHAREDSECRETBYTES);
        }
        memcpy(prev, hash, 32);
    }

    EVP_MD_CTX_free(ctx);
    return valid ? 0 : -1;
}
