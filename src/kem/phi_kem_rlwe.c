// FEmmg-FHE — φ-LWE KEM: Ring-LWE in the φ-extension ring
// Ring = Z_q[X]/(X²-X-1) × Z_q[Y]/(Y^N+1)
// Secret = a + bφ (2 coefficients). Tiny keys. φ-native structure.
// Security: Ring-LWE in product ring. Compact. Elegant.

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// φ-extension parameters
#define PHI_Q 3329       // 12-bit modulus
#define PHI_N 256        // cyclotomic ring dimension
#define PHI_D 2          // φ-extension degree (a + bφ)

// Derived sizes
#define PHI_COEFF_BYTES 2   // 12-bit fits in 2 bytes
#define PHI_POLY_BYTES (PHI_N * PHI_COEFF_BYTES)  // 512 bytes per polynomial

// Key sizes (φ-extension: 2 polynomials per element)
#define PHI_SK_BYTES (PHI_D * PHI_POLY_BYTES)  // 1024 bytes
#define PHI_PK_BYTES (PHI_D * PHI_POLY_BYTES + 32)  // 1056 bytes
#define PHI_CT_BYTES (PHI_D * PHI_POLY_BYTES + PHI_POLY_BYTES + 32)  // 1568 bytes
#define PHI_SS_BYTES 32

// ============================================
// LITTLE POLY MATH (mod Q, mod X^N+1)
// ============================================

static int16_t csubq(int16_t a) { a -= PHI_Q; a += (a >> 15) & PHI_Q; return a; }

static void poly_add(int16_t *r, const int16_t *a, const int16_t *b) {
    for (int i = 0; i < PHI_N; i++) r[i] = csubq(a[i] + b[i]);
}
static void poly_sub(int16_t *r, const int16_t *a, const int16_t *b) {
    for (int i = 0; i < PHI_N; i++) r[i] = csubq(a[i] - b[i] + PHI_Q);
}

// Schoolbook multiply mod X^N+1, mod Q
static void poly_mul(int16_t *r, const int16_t *a, const int16_t *b) {
    int32_t tmp[2*PHI_N];
    memset(tmp, 0, sizeof(tmp));
    for (int i = 0; i < PHI_N; i++)
        for (int j = 0; j < PHI_N; j++)
            tmp[i+j] += (int32_t)a[i] * b[j];
    for (int i = 0; i < PHI_N; i++) {
        int32_t v = tmp[i] - tmp[i+PHI_N];
        v %= PHI_Q; if (v < 0) v += PHI_Q;
        r[i] = (int16_t)v;
    }
}

// Tiny noise: 2-bit centered (rejection-free)
static void tiny_noise(int16_t *r, int len) {
    for (int i = 0; i < len; i++) {
        uint8_t bits;
        RAND_bytes(&bits, 1);
        int a = (bits & 1) + ((bits>>2)&1);
        int b = ((bits>>4)&1) + ((bits>>6)&1);
        r[i] = (a - b + PHI_Q) % PHI_Q;
    }
}

// Sample uniform polynomial
static void uniform_poly(int16_t *r) {
    for (int i = 0; i < PHI_N; i++) {
        uint8_t bytes[2];
        RAND_bytes(bytes, 2);
        r[i] = ((bytes[0] | (bytes[1] << 8)) & 0xFFF) % PHI_Q;
    }
}

// ============================================
// φ-EXTENSION ELEMENT: a + bφ
// Stored as [a, b] each a polynomial of N coefficients
// ============================================

// φ-multiply in the ring: (a+bφ)(c+dφ) = (ac+bd) + (ad+bc+bd)φ
static void phi_mul(int16_t *r_a, int16_t *r_b,
                     const int16_t *a_a, const int16_t *a_b,
                     const int16_t *b_a, const int16_t *b_b) {
    int16_t ac[PHI_N], bd[PHI_N], ad[PHI_N], bc[PHI_N];
    poly_mul(ac, a_a, b_a);
    poly_mul(bd, a_b, b_b);
    poly_mul(ad, a_a, b_b);
    poly_mul(bc, a_b, b_a);
    // real = ac + bd
    poly_add(r_a, ac, bd);
    // φ-part = ad + bc + bd
    int16_t tmp[PHI_N];
    poly_add(tmp, ad, bc);
    poly_add(r_b, tmp, bd);
}

// ============================================
// KEY GENERATION
// ============================================
int phi_kem_keygen(uint8_t *pk, uint8_t *sk) {
    // Secret: random small polynomial pair (s_a, s_b)
    int16_t s_a[PHI_N], s_b[PHI_N];
    tiny_noise(s_a, PHI_N);
    tiny_noise(s_b, PHI_N);
    
    // Public generator: uniform A = (A_a, A_b)
    int16_t A_a[PHI_N], A_b[PHI_N];
    uniform_poly(A_a);
    uniform_poly(A_b);
    
    // Error: small
    int16_t e_a[PHI_N], e_b[PHI_N];
    tiny_noise(e_a, PHI_N);
    tiny_noise(e_b, PHI_N);
    
    // Public key: T = A·S + E (φ-multiplication)
    int16_t AS_a[PHI_N], AS_b[PHI_N];
    phi_mul(AS_a, AS_b, A_a, A_b, s_a, s_b);
    int16_t T_a[PHI_N], T_b[PHI_N];
    poly_add(T_a, AS_a, e_a);
    poly_add(T_b, AS_b, e_b);
    
    // Encode sk = (s_a, s_b)
    for (int i = 0; i < PHI_N; i++) {
        sk[i*2] = s_a[i] & 0xFF;
        sk[i*2+1] = (s_a[i] >> 8) & 0xFF;
        sk[PHI_POLY_BYTES + i*2] = s_b[i] & 0xFF;
        sk[PHI_POLY_BYTES + i*2+1] = (s_b[i] >> 8) & 0xFF;
    }
    
    // Encode pk = (A_a, A_b, T_a, T_b) — but A can be regenerated from seed!
    // Compact: store seed(32) + T
    uint8_t seed[32];
    RAND_bytes(seed, 32);
    memcpy(pk, seed, 32);
    for (int i = 0; i < PHI_N; i++) {
        pk[32 + i*2] = T_a[i] & 0xFF;
        pk[32 + i*2+1] = (T_a[i] >> 8) & 0xFF;
        pk[32 + PHI_POLY_BYTES + i*2] = T_b[i] & 0xFF;
        pk[32 + PHI_POLY_BYTES + i*2+1] = (T_b[i] >> 8) & 0xFF;
    }
    
    return 0;
}

// ============================================
// ENCAPSULATION
// ============================================
int phi_kem_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    // Recover A from seed
    uint8_t seed[32];
    memcpy(seed, pk, 32);
    int16_t A_a[PHI_N], A_b[PHI_N];
    // Use seed to regenerate A (simplified: hash-based)
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_shake128(), NULL);
    EVP_DigestUpdate(ctx, seed, 32);
    uint8_t A_bytes[PHI_D * PHI_POLY_BYTES];
    EVP_DigestFinalXOF(ctx, A_bytes, sizeof(A_bytes));
    EVP_MD_CTX_free(ctx);
    for (int i = 0; i < PHI_N; i++) {
        A_a[i] = (A_bytes[i*2] | (A_bytes[i*2+1] << 8)) % PHI_Q;
        A_b[i] = (A_bytes[PHI_POLY_BYTES + i*2] | (A_bytes[PHI_POLY_BYTES + i*2+1] << 8)) % PHI_Q;
    }
    
    // Parse T from pk
    int16_t T_a[PHI_N], T_b[PHI_N];
    for (int i = 0; i < PHI_N; i++) {
        T_a[i] = pk[32 + i*2] | (pk[32 + i*2+1] << 8);
        T_b[i] = pk[32 + PHI_POLY_BYTES + i*2] | (pk[32 + PHI_POLY_BYTES + i*2+1] << 8);
    }
    
    // Random message
    uint8_t m[32];
    RAND_bytes(m, 32);
    
    // Random vector R (small)
    int16_t r_a[PHI_N], r_b[PHI_N];
    tiny_noise(r_a, PHI_N);
    tiny_noise(r_b, PHI_N);
    
    // Error terms
    int16_t e1_a[PHI_N], e1_b[PHI_N], e2_a[PHI_N], e2_b[PHI_N];
    tiny_noise(e1_a, PHI_N); tiny_noise(e1_b, PHI_N);
    tiny_noise(e2_a, PHI_N); tiny_noise(e2_b, PHI_N);
    
    // U = A·R + E1 (φ-multiplication)
    int16_t AR_a[PHI_N], AR_b[PHI_N];
    phi_mul(AR_a, AR_b, A_a, A_b, r_a, r_b);
    int16_t U_a[PHI_N], U_b[PHI_N];
    poly_add(U_a, AR_a, e1_a);
    poly_add(U_b, AR_b, e1_b);
    
    // V = T·R + E2 + encode(m)
    int16_t TR_a[PHI_N], TR_b[PHI_N];
    phi_mul(TR_a, TR_b, T_a, T_b, r_a, r_b);
    // Encode m into the top bits of V
    int16_t V_a[PHI_N], V_b[PHI_N];
    for (int i = 0; i < PHI_N; i++) {
        int bit = (i < 256) ? ((m[i/8] >> (i%8)) & 1) : 0;
        V_a[i] = csubq(TR_a[i] + e2_a[i] + bit * (PHI_Q/2));
        V_b[i] = csubq(TR_b[i] + e2_b[i]);
    }
    
    // Encode ct = (U_a, U_b, V_a)
    for (int i = 0; i < PHI_N; i++) {
        ct[i*2] = U_a[i] & 0xFF;
        ct[i*2+1] = (U_a[i] >> 8) & 0xFF;
        ct[PHI_POLY_BYTES + i*2] = U_b[i] & 0xFF;
        ct[PHI_POLY_BYTES + i*2+1] = (U_b[i] >> 8) & 0xFF;
        ct[2*PHI_POLY_BYTES + i*2] = V_a[i] & 0xFF;
        ct[2*PHI_POLY_BYTES + i*2+1] = (V_a[i] >> 8) & 0xFF;
    }
    
    // Shared secret: H(m)
    ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, m, 32);
    unsigned int len;
    EVP_DigestFinal_ex(ctx, ss, &len);
    EVP_MD_CTX_free(ctx);
    
    return 0;
}

// ============================================
// DECAPSULATION
// ============================================
int phi_kem_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk) {
    // Parse secret
    int16_t s_a[PHI_N], s_b[PHI_N];
    for (int i = 0; i < PHI_N; i++) {
        s_a[i] = sk[i*2] | (sk[i*2+1] << 8);
        s_b[i] = sk[PHI_POLY_BYTES + i*2] | (sk[PHI_POLY_BYTES + i*2+1] << 8);
    }
    
    // Parse ct
    int16_t U_a[PHI_N], U_b[PHI_N], V_a[PHI_N];
    for (int i = 0; i < PHI_N; i++) {
        U_a[i] = ct[i*2] | (ct[i*2+1] << 8);
        U_b[i] = ct[PHI_POLY_BYTES + i*2] | (ct[PHI_POLY_BYTES + i*2+1] << 8);
        V_a[i] = ct[2*PHI_POLY_BYTES + i*2] | (ct[2*PHI_POLY_BYTES + i*2+1] << 8);
    }
    
    // Recover: V - U·S
    int16_t US_a[PHI_N], US_b[PHI_N];
    phi_mul(US_a, US_b, U_a, U_b, s_a, s_b);
    int16_t M_a[PHI_N];
    poly_sub(M_a, V_a, US_a);
    
    // Decode message
    uint8_t m[32];
    memset(m, 0, 32);
    for (int i = 0; i < 256; i++) {
        M_a[i] = csubq(M_a[i]);
        if (M_a[i] > PHI_Q/4 && M_a[i] < 3*PHI_Q/4)
            m[i/8] |= (1 << (i%8));
    }
    
    // Derive shared secret
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, m, 32);
    unsigned int len;
    EVP_DigestFinal_ex(ctx, ss, &len);
    EVP_MD_CTX_free(ctx);
    
    return 0;
}

// ============================================
// TEST
// ============================================
int main() {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-LWE KEM: Ring-LWE in the φ-extension ring          ║\n");
    printf(  "  ║   Z_q[X]/(X²-X-1) × Z_q[Y]/(Y^%d+1)                     ║\n", PHI_N);
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  q=%d, N=%d, φ-degree=%d\n", PHI_Q, PHI_N, PHI_D);
    printf("  SK=%dB  PK=%dB  CT=%dB  SS=%dB\n", 
           PHI_SK_BYTES, PHI_PK_BYTES, PHI_CT_BYTES, PHI_SS_BYTES);
    printf("  Total: %dB (Kyber-512: 3200B)\n", 
           PHI_SK_BYTES+PHI_PK_BYTES+PHI_CT_BYTES);
    printf("  Key is φ-structured: s = s_a + s_b·φ\n\n");

    int passed = 0, total = 30;
    printf("  Testing %d exchanges... " , total); fflush(stdout);
    
    for (int i = 0; i < total; i++) {
        uint8_t pk[PHI_PK_BYTES], sk[PHI_SK_BYTES], ct[PHI_CT_BYTES], ss1[PHI_SS_BYTES], ss2[PHI_SS_BYTES];
        phi_kem_keygen(pk, sk);
        phi_kem_encaps(ct, ss1, pk);
        phi_kem_decaps(ss2, ct, sk);
        if (memcmp(ss1, ss2, PHI_SS_BYTES) == 0) passed++;
    }
    
    printf("%d/%d passed\n\n", passed, total);
    
    if (passed == total) {
        printf("  ✅ φ-LWE KEM WORKING\n");
        printf("  φ-structured keys. Ring-LWE security. Compact.\n\n");
    } else {
        printf("  ❌ %d/%d — needs debugging\n\n", passed, total);
    }
    
    printf("  I AM THAT I AM\n\n");
    return (passed == total) ? 0 : 1;
}
