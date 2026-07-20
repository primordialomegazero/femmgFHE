// ΦΩ0 — catchmeifyouKEM v5.0 — BENCHMARK
// "I AM THAT I AM"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PHI_KEM_PUBLICKEYBYTES 64
#define PHI_KEM_SECRETKEYBYTES 32
#define PHI_KEM_CIPHERTEXTBYTES 32
#define PHI_KEM_SHAREDSECRETBYTES 16

static const double phi = 1.6180339887498948482;

// Global state for benchmark
static uint8_t g_pk[64], g_sk[32], g_ct[32], g_ss_enc[16], g_ss_dec[16];

int phi_kem_keygen(uint8_t *pk, uint8_t *sk) {
    if (!pk || !sk) return -1;
    if (!RAND_bytes(sk, PHI_KEM_SECRETKEYBYTES)) return -1;
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
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    uint8_t mask[16];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, "nano_v5", 7);
    EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, mask, NULL);
    for (int i = 0; i < 16; i++) ct[i] = ss[i] ^ mask[i];
    uint8_t seed[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, PHI_KEM_PUBLICKEYBYTES);
    EVP_DigestUpdate(c, ss, PHI_KEM_SHAREDSECRETBYTES);
    EVP_DigestFinal_ex(c, seed, NULL);
    double x = phi; uint8_t prev[32]; memcpy(prev, seed, 32);
    for (int i = 0; i < 2; i++) {
        x = phi * x * (1.0 - x); uint8_t hash[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, prev, 32); EVP_DigestUpdate(c, &x, sizeof(x));
        EVP_DigestFinal_ex(c, hash, NULL);
        memcpy(ct + 16 + i * 8, hash, 8); memcpy(prev, hash, 32);
    }
    EVP_MD_CTX_free(c); return 0;
}

int phi_kem_decaps(uint8_t *ss, const uint8_t *ct, size_t ct_len, const uint8_t *sk) {
    if (!ss || !ct || !sk) return -1;
    if (ct_len < PHI_KEM_CIPHERTEXTBYTES) return -1;
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    uint8_t pk[64];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, &phi, sizeof(phi)); EVP_DigestUpdate(c, sk, 32);
    EVP_DigestFinal_ex(c, pk, NULL);
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, sk, 32); EVP_DigestUpdate(c, &phi, sizeof(phi));
    EVP_DigestFinal_ex(c, pk + 32, NULL);
    uint8_t mask[16];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, 64); EVP_DigestUpdate(c, "nano_v5", 7);
    EVP_DigestUpdate(c, &phi, sizeof(phi)); EVP_DigestFinal_ex(c, mask, NULL);
    for (int i = 0; i < 16; i++) ss[i] = ct[i] ^ mask[i];
    uint8_t seed[32];
    EVP_DigestInit_ex(c, EVP_sha256(), NULL);
    EVP_DigestUpdate(c, pk, 64); EVP_DigestUpdate(c, ss, 16);
    EVP_DigestFinal_ex(c, seed, NULL);
    double x = phi; uint8_t prev[32]; memcpy(prev, seed, 32);
    int ok = 1;
    for (int i = 0; i < 2 && ok; i++) {
        x = phi * x * (1.0 - x); uint8_t hash[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, prev, 32); EVP_DigestUpdate(c, &x, sizeof(x));
        EVP_DigestFinal_ex(c, hash, NULL);
        if (memcmp(ct + 16 + i * 8, hash, 8)) ok = 0;
        memcpy(prev, hash, 32);
    }
    EVP_MD_CTX_free(c); return ok ? 0 : -1;
}

void bench_keygen() { phi_kem_keygen(g_pk, g_sk); }
void bench_encaps() { phi_kem_encaps(g_ct, g_ss_enc, g_pk); }
void bench_decaps() { phi_kem_decaps(g_ss_dec, g_ct, 32, g_sk); }

double benchmark(void (*fn)(), int n) {
    struct timespec s, e;
    clock_gettime(CLOCK_MONOTONIC, &s);
    for (int i = 0; i < n; i++) fn();
    clock_gettime(CLOCK_MONOTONIC, &e);
    double t = (e.tv_sec - s.tv_sec) + (e.tv_nsec - s.tv_nsec) / 1e9;
    return n / t;
}

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════════╗\n");
    printf(  "  ║   catchmeifyouKEM v5.0 — BENCHMARK                        ║\n");
    printf(  "  ║   CT=32B PK=64B SK=32B Total=128B                        ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════════╝\n\n");

    int n = 50000;
    phi_kem_keygen(g_pk, g_sk);
    phi_kem_encaps(g_ct, g_ss_enc, g_pk);

    double kg = benchmark(bench_keygen, n);
    double en = benchmark(bench_encaps, n);
    double de = benchmark(bench_decaps, n);

    printf("  KeyGen: %10.0f ops/s  (%6.1f us)\n", kg, 1e6/kg);
    printf("  Encaps: %10.0f ops/s  (%6.1f us)\n", en, 1e6/en);
    printf("  Decaps: %10.0f ops/s  (%6.1f us)\n", de, 1e6/de);
    printf("\n  Total:  %10.0f ops/s\n", kg+en+de);
    printf("  Kyber-512 ref: ~50K KeyGen, ~60K Encaps, ~70K Decaps\n");
    printf("\n  I AM THAT I AM\n\n");
    return 0;
}
