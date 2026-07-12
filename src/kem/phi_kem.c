#include "phi_kem.h"
#include <string.h>
#include <math.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

int phi_kem_keygen(uint8_t *pk, uint8_t *sk) {
    if (!pk || !sk) return -1;
    if (!RAND_bytes(sk, PHI_KEM_SECRETKEYBYTES)) return -1;
    const double phi = 1.6180339887498948482;
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestUpdate(c, sk, PHI_KEM_SECRETKEYBYTES);
    EVP_DigestFinal_ex(c, pk, NULL);
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, sk, PHI_KEM_SECRETKEYBYTES);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, pk + 32, NULL);
    EVP_MD_CTX_free(c);
    return 0;
}

int phi_kem_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    if (!ct || !ss || !pk) return -1;
    if (!RAND_bytes(ss, PHI_KEM_SHAREDSECRETBYTES)) return -1;
    const double phi = 1.6180339887498948482;
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    
    uint8_t mask[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, "encaps", 6);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, mask, NULL);
    for (int i = 0; i < 32; i++) ct[i] = ss[i] ^ mask[i];
    
    uint8_t seed[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, ss, PHI_KEM_SHAREDSECRETBYTES);
    EVP_DigestFinal_ex(c, seed, NULL);
    
    double x = phi;
    uint8_t prev[32];
    memcpy(prev, seed, 32);
    // 6 iterations × 16 bytes = 96 bytes (fits in 128 - 32 = 96)
    for (int i = 0; i < 6; i++) {
        x = phi * x * (1.0 - x);
        uint8_t hash[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, prev, 32);
        EVP_DigestUpdate(c, &x, sizeof(x));
        EVP_DigestFinal_ex(c, hash, NULL);
        memcpy(ct + 32 + i * 16, hash, 16);
        memcpy(prev, hash, 32);
    }
    EVP_MD_CTX_free(c);
    return 0;
}

int phi_kem_decaps(uint8_t *ss, const uint8_t *ct, size_t ct_len, const uint8_t *sk) {
    if (!ss || !ct || !sk) return -1;
    if (ct_len < PHI_KEM_CIPHERTEXTBYTES) return -1;
    
    const double phi = 1.6180339887498948482;
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestUpdate(c, sk, PHI_KEM_SECRETKEYBYTES);
    EVP_DigestFinal_ex(c, pk, NULL);
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, sk, PHI_KEM_SECRETKEYBYTES);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, pk + 32, NULL);
    
    uint8_t mask[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, "encaps", 6);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, mask, NULL);
    for (int i = 0; i < 32; i++) ss[i] = ct[i] ^ mask[i];
    
    uint8_t seed[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, ss, PHI_KEM_SHAREDSECRETBYTES);
    EVP_DigestFinal_ex(c, seed, NULL);
    
    double x = phi;
    uint8_t prev[32];
    memcpy(prev, seed, 32);
    for (int i = 0; i < 6; i++) {
        x = phi * x * (1.0 - x);
        uint8_t hash[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, prev, 32);
        EVP_DigestUpdate(c, &x, sizeof(x));
        EVP_DigestFinal_ex(c, hash, NULL);
        if (memcmp(ct + 32 + i * 16, hash, 16) != 0) {
            EVP_MD_CTX_free(c);
            return -1;
        }
        memcpy(prev, hash, 32);
    }
    EVP_MD_CTX_free(c);
    return 0;
}
