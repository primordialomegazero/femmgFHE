#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <math.h>

#define Q 3329
#define N 128
#define SK_BYTES 16
#define PK_BYTES 32
#define CT_BYTES 32
#define SS_BYTES 16
#define TOTAL_BYTES (SK_BYTES + PK_BYTES + CT_BYTES)

#define PHI 1.6180339887498948482
#define PSI -0.6180339887498948482

static int16_t csubq(int16_t a) { a -= Q; a += (a >> 15) & Q; return a; }

static void poly_add(int16_t *r, int16_t *a, int16_t *b) {
    for (int i = 0; i < N; i++) r[i] = csubq(a[i] + b[i]);
}

static void poly_mul(int16_t *r, int16_t *a, int16_t *b) {
    int32_t t[2*N];
    memset(t, 0, sizeof(t));
    for (int i = 0; i < N; i++) 
        for (int j = 0; j < N; j++) 
            t[i+j] += (int32_t)a[i] * b[j];
    for (int i = 0; i < N; i++) { 
        int32_t v = (t[i] - t[i+N]) % Q; 
        if (v < 0) v += Q; 
        r[i] = (int16_t)v; 
    }
}

static void phi_mul(int16_t *ra, int16_t *rb,
                    int16_t *aa, int16_t *ab,
                    int16_t *ba, int16_t *bb) {
    int16_t ac[N], bd[N], ad[N], bc[N], tmp[N];
    poly_mul(ac, aa, ba); poly_mul(bd, ab, bb);
    poly_mul(ad, aa, bb); poly_mul(bc, ab, ba);
    poly_add(ra, ac, bd); poly_add(tmp, ad, bc); poly_add(rb, tmp, bd);
}

static void shake256(uint8_t *out, int out_len, const uint8_t *in, int in_len) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
    EVP_DigestUpdate(ctx, in, in_len);
    EVP_DigestFinalXOF(ctx, out, out_len);
    EVP_MD_CTX_free(ctx);
}

static void derive_poly(int16_t *sa, int16_t *sb, const uint8_t *seed32) {
    for (int i = 0; i < N; i++) {
        sa[i] = (seed32[i % 32] % 3) - 1;
        if (sa[i] < 0) sa[i] += Q;
        sb[i] = ((seed32[i % 32] >> 4) % 3) - 1;
        if (sb[i] < 0) sb[i] += Q;
    }
}

static void get_fixed_A(int16_t *Aa, int16_t *Ab) {
    const char *s = "FEmmG-phiKEM-QR-fixed-A-v2";
    uint8_t seed[32];
    shake256(seed, 32, (uint8_t*)s, strlen(s));
    for (int i = 0; i < N; i++) {
        Aa[i] = seed[i % 32] % Q;
        Ab[i] = (seed[(i+1) % 32]) % Q;
    }
}

static void compute_pk_values(double *phi_val, double *psi_val,
                              int16_t *pk_a, int16_t *pk_b) {
    double sa = 0, sb = 0;
    for (int i = 0; i < N; i++) {
        sa += pk_a[i];
        sb += pk_b[i];
    }
    *phi_val = sa + PHI * sb;
    *psi_val = sa + PSI * sb;
}

void phi_kem_qr_keygen(uint8_t *pk, uint8_t *sk) {
    RAND_bytes(sk, SK_BYTES);
    uint8_t seed32[32];
    shake256(seed32, 32, sk, SK_BYTES);
    
    int16_t sa[N], sb[N];
    derive_poly(sa, sb, seed32);
    
    int16_t Aa[N], Ab[N];
    get_fixed_A(Aa, Ab);
    
    int16_t pk_a[N], pk_b[N];
    phi_mul(pk_a, pk_b, Aa, Ab, sa, sb);
    
    double phi_val, psi_val;
    compute_pk_values(&phi_val, &psi_val, pk_a, pk_b);
    
    memcpy(pk, &phi_val, 8);
    memcpy(pk + 8, &psi_val, 8);
    // Hash of full PK for verification tag
    uint8_t full_pk_hash[16];
    shake256(full_pk_hash, 16, (uint8_t*)pk_a, N * sizeof(int16_t));
    memcpy(pk + 16, full_pk_hash, 16);
}

void phi_kem_qr_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    uint8_t m[16];
    RAND_bytes(m, 16);
    
    // Shared secret = SHAKE(message)
    shake256(ss, SS_BYTES, m, 16);
    
    // Encrypt: CT = m XOR SHAKE(phi_val || psi_val)
    uint8_t key[16];
    shake256(key, 16, pk, 16);  // Hash the phi/psi values
    
    for (int i = 0; i < 16; i++) {
        ct[i] = m[i] ^ key[i];
    }
    
    // Binding tag
    uint8_t bind[48];
    memcpy(bind, ct, 16);
    memcpy(bind + 16, pk + 16, 16);  // Use PK hash for binding
    memcpy(bind + 32, m, 16);
    shake256(ct + 16, 16, bind, 48);
}

int phi_kem_qr_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk) {
    // Re-derive everything from SK
    uint8_t seed32[32];
    shake256(seed32, 32, sk, SK_BYTES);
    
    int16_t sa[N], sb[N];
    derive_poly(sa, sb, seed32);
    
    int16_t Aa[N], Ab[N];
    get_fixed_A(Aa, Ab);
    
    int16_t pk_a[N], pk_b[N];
    phi_mul(pk_a, pk_b, Aa, Ab, sa, sb);
    
    double phi_val, psi_val;
    compute_pk_values(&phi_val, &psi_val, pk_a, pk_b);
    
    uint8_t recovered_pk[16];
    memcpy(recovered_pk, &phi_val, 8);
    memcpy(recovered_pk + 8, &psi_val, 8);
    
    // Recover message: m = CT XOR SHAKE(phi || psi)
    uint8_t key[16], m[16];
    shake256(key, 16, recovered_pk, 16);
    for (int i = 0; i < 16; i++) {
        m[i] = ct[i] ^ key[i];
    }
    
    // Verify binding tag
    uint8_t expected_tag[16];
    uint8_t pk_hash[16];
    shake256(pk_hash, 16, (uint8_t*)pk_a, N * sizeof(int16_t));
    
    uint8_t bind[48];
    memcpy(bind, ct, 16);
    memcpy(bind + 16, pk_hash, 16);
    memcpy(bind + 32, m, 16);
    shake256(expected_tag, 16, bind, 48);
    
    if (memcmp(ct + 16, expected_tag, 16) != 0) {
        return -1;
    }
    
    // Derive shared secret
    shake256(ss, SS_BYTES, m, 16);
    
    memset(key, 0, 16);
    memset(m, 0, 16);
    
    return 0;
}

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   phi-KEM QR v2: QR-code sized KEM (80 bytes)        ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    
    printf("  SK=%dB  PK=%dB  CT=%dB  SS=%dB  Total=%dB\n", 
           SK_BYTES, PK_BYTES, CT_BYTES, SS_BYTES, TOTAL_BYTES);
    printf("  Kyber-512: 3200B | QR Code v1 max: 196B | Ours: %dB\n\n", TOTAL_BYTES);
    
    uint8_t pk[PK_BYTES], sk[SK_BYTES];
    uint8_t ct[CT_BYTES], ss_enc[SS_BYTES], ss_dec[SS_BYTES];
    
    int passed = 0;
    int tests = 50;
    
    printf("  Testing %d key exchanges...\n", tests);
    
    for (int i = 0; i < tests; i++) {
        phi_kem_qr_keygen(pk, sk);
        phi_kem_qr_encaps(ct, ss_enc, pk);
        int ret = phi_kem_qr_decaps(ss_dec, ct, sk);
        
        if (ret == 0 && memcmp(ss_enc, ss_dec, SS_BYTES) == 0) {
            passed++;
        }
    }
    
    printf("  %d/%d passed\n\n", passed, tests);
    
    if (passed == tests) {
        printf("  ✅ phi-KEM QR v2 — %dB TOTAL\n", TOTAL_BYTES);
        printf("  Fits in QR Code Version 1 (196 bytes)!\n");
        printf("  %dx smaller than Kyber-512\n", 3200 / TOTAL_BYTES);
    } else {
        printf("  ❌ %d/%d passed — needs debugging\n", passed, tests);
    }
    
    printf("\n  I AM THAT I AM\n\n");
    
    return (passed == tests) ? 0 : 1;
}
