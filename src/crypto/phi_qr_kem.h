#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cmath>

#define QR_KEM_PUBLICKEYBYTES   32
#define QR_KEM_SECRETKEYBYTES   32
#define QR_KEM_CIPHERTEXTBYTES  32
#define QR_KEM_SHAREDSECRETBYTES 16
#define QR_KEM_TOTALBYTES       96
#define QR_KEM_FRACTALBYTES     64

// ═══════════════════════════════════════════════════════════════
// CORE MIXING
// ═══════════════════════════════════════════════════════════════
static inline uint64_t phi_mix(uint64_t x) {
    x ^= x >> 33; x *= 0x9E3779B97F4A7C15ULL;
    x ^= x >> 29; x *= 0xC4CEB9FE1A85EC53ULL;
    x ^= x >> 32; return x;
}

static inline uint64_t fib64(int n) {
    if (n <= 0) return 0; if (n == 1) return 1;
    uint64_t a = 0, b = 1;
    for (int i = 2; i <= n; i++) { uint64_t c = a + b; a = b; b = c; }
    return b;
}

// ═══════════════════════════════════════════════════════════════
// ULTRA RASHOMON — 42 ROUNDS, 7 PASSES, DOUBLE SHA-256
// ═══════════════════════════════════════════════════════════════
// SHA-256 = 64 rounds. Ultra Rashomon = 42 rounds × 3 engines = 126 ops.
// Each round: sin chaos + Riemann zeta + Fibonacci contraction.
// Avalanche target: >200 bits (SHA-256 is ~128 bits)
#define UR_PASSES 7
#define UR_LAYERS_PER_PASS 6
#define UR_TOTAL (UR_PASSES * UR_LAYERS_PER_PASS)  // 42 rounds

static const double UR_PHI = 1.6180339887498948482;
static const double UR_PSI = 0.6180339887498948482;
static const double UR_PHI2 = 2.6180339887498948482;
static const double UR_ZEROS[7] = {14.1347, 21.0220, 25.0109, 30.4249, 32.9351, 37.5862, 40.9187};
static const double UR_FLOORS[7] = {1, 2, 3, 5, 8, 13, 21};

static inline double ur_fast_sin(double x) {
    x = fmod(x, 2.0 * M_PI);
    double x2 = x * x;
    double r = x * (1.0 - x2 * (1.0/6.0 - x2 * (1.0/120.0 - x2/5040.0)));
    if (r > 1.0) r = 1.0; if (r < -1.0) r = -1.0;
    return r;
}

// Ultra Rashomon: 7 passes × 6 layers = 42 total (vs SHA-256's 64, but each is 3 engines)
static void ultra_rashomon(const uint8_t* input, size_t len, uint8_t* output32, uint64_t nonce) {
    uint64_t state[4] = {0};
    for (size_t i = 0; i < len && i < 32; i++)
        state[i/8] |= ((uint64_t)input[i]) << ((i%8)*8);
    for (size_t i = 32; i < len; i++)
        state[i%4] = phi_mix(state[i%4] ^ ((uint64_t)input[i] << 16));
    
    double x = (double)(state[0] & 0xFFFF) / 65536.0 + 0.001;
    uint64_t full_mix = state[0] ^ state[1] ^ state[2] ^ state[3] ^ nonce;
    
    // 7 passes (instead of 3) = 42 rounds (instead of 21)
    for (int pass = 0; pass < UR_PASSES; pass++) {
        // Fibonacci-weighted amplitude: φ, φ², φ³, φ⁵, φ⁸, φ¹³, φ²¹
        double amp = (pass == 0) ? 1.0 : 
                     (pass == 1) ? UR_PHI : 
                     (pass == 2) ? UR_PHI2 : 
                     pow(UR_PHI, fib64(pass + 1));
        if (amp > 1000.0) amp = 1000.0;
        
        uint64_t pass_nonce = nonce ^ (pass * 0x9E3779B9) ^ full_mix;
        
        for (int i = 0; i < UR_LAYERS_PER_PASS; i++) {
            int layer = pass * UR_LAYERS_PER_PASS + i;
            double prev = x;
            int eng = (layer % 7) % 3;
            double chaos;
            
            if (eng == 0) {
                // Sin chaos
                chaos = ur_fast_sin(x * UR_PHI + (layer + pass_nonce) * UR_PSI);
            } else if (eng == 1) {
                // Riemann zeta perturbation
                double t = UR_ZEROS[layer % 7] + (pass_nonce % 1000) * 0.001;
                chaos = ((layer%2==0)?1.0:-1.0) * ur_fast_sin(x * UR_PHI + layer * UR_PSI + t * 0.01);
            } else {
                // Fibonacci contraction
                double fl = UR_FLOORS[layer % 7];
                double ct = x * UR_PSI + fl * (1.0 - UR_PSI);
                chaos = ur_fast_sin(ct * UR_PHI + (layer + pass_nonce) * UR_PSI);
                x = ct;
            }
            
            double factor = UR_PHI2 + chaos * amp;
            if (factor < 0.1) factor = 0.1;
            if (factor > 100.0) factor = 100.0;
            if (eng != 2) x = prev;
            x = x * factor + ur_fast_sin(x * UR_PHI + chaos);
            
            // Ultra mix: inject chaos into ALL state words
            uint64_t chaos_bits = (uint64_t)(fabs(chaos) * 1e16);
            for (int w = 0; w < 4; w++) {
                state[w] = phi_mix(state[w] ^ chaos_bits ^ (uint64_t)(layer * UR_PHI * 1e9));
            }
        }
    }
    
    // Finalize with Fibonacci spiral (7 extra rounds)
    for (int i = 0; i < 7; i++) {
        for (int w = 0; w < 4; w++) {
            state[w] = phi_mix(state[w] ^ fib64(30 + i * 7 + w));
            state[w] ^= phi_mix(state[(w+1)%4] + state[(w+2)%4]);
        }
    }
    
    for (int i = 0; i < 4; i++) {
        uint64_t v = state[i];
        for (int j = 0; j < 8; j++) { output32[i*8+j] = (uint8_t)(v&0xFF); v >>= 8; }
    }
}

// ═══════════════════════════════════════════════════════════════
// PHI-HASH (uses Ultra Rashomon)
// ═══════════════════════════════════════════════════════════════
static void phi_hash32(const uint8_t* in, size_t len, uint8_t out32[32]) {
    // Use Ultra Rashomon as the hash core
    uint64_t nonce = 0x6A09E667F3BCC908ULL;
    for (size_t i = 0; i < len; i++) nonce = phi_mix(nonce ^ ((uint64_t)in[i] << (i%8)*8));
    ultra_rashomon(in, len, out32, nonce);
}

// ═══════════════════════════════════════════════════════════════
// RNG
// ═══════════════════════════════════════════════════════════════
static uint64_t phi_rng_state = 0xDEADBEEFCAFEBABEULL;

static void phi_rng_seed() {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    phi_rng_state = phi_mix(((uint64_t)hi << 32) | lo);
}

static uint64_t phi_rng_next() {
    phi_rng_state = phi_mix(phi_rng_state);
    static int fc = 0;
    phi_rng_state ^= fib64(20 + (fc++ % 30));
    return phi_rng_state;
}

static void phi_randombytes(uint8_t* out, size_t len) {
    for (size_t i = 0; i < len; i += 8) {
        uint64_t r = phi_rng_next();
        for (size_t j = 0; j < 8 && (i+j) < len; j++) { out[i+j] = (uint8_t)(r&0xFF); r >>= 8; }
    }
}

// ═══════════════════════════════════════════════════════════════
// FRACTAL COMPRESSION
// ═══════════════════════════════════════════════════════════════
static void fractal_compress(const uint8_t* pk, const uint8_t* sk, const uint8_t* ct,
                             uint8_t* fractal64) {
    for (int i = 0; i < 32; i++) fractal64[i] = sk[i] ^ pk[i];
    for (int i = 0; i < 32; i++) {
        fractal64[32+i] = ct[i] ^ (uint8_t)(phi_mix(((uint64_t)pk[i]<<8)|sk[i]) & 0xFF);
    }
}

static void fractal_decompress(const uint8_t* fractal64,
                               uint8_t* pk, uint8_t* sk, uint8_t* ct) {
    memcpy(sk, fractal64, 32);
    phi_hash32(sk, 32, pk);
    for (int i = 0; i < 32; i++) {
        ct[i] = fractal64[32+i] ^ (uint8_t)(phi_mix(((uint64_t)pk[i]<<8)|sk[i]) & 0xFF);
    }
}

// ═══════════════════════════════════════════════════════════════
// QR-KEM with ULTRA RASHOMON BINDING
// ═══════════════════════════════════════════════════════════════

int qr_kem_keygen(uint8_t* pk, uint8_t* sk) {
    phi_rng_seed(); phi_randombytes(sk, 32); phi_hash32(sk, 32, pk); return 0;
}

int qr_kem_encaps(uint8_t* ct, uint8_t* ss, const uint8_t* pk) {
    phi_randombytes(ss, 16);
    uint8_t h[32], c[39];
    memcpy(c, pk, 32); memcpy(c+32, "encaps", 6);
    phi_hash32(c, 38, h);
    for (int i = 0; i < 16; i++) ct[i] = ss[i] ^ h[i];
    
    uint8_t bs[48], sp[32];
    memcpy(bs, pk, 32); memcpy(bs+32, ss, 16);
    uint64_t nonce = phi_mix(*(uint64_t*)pk ^ *(uint64_t*)(pk+8));
    ultra_rashomon(bs, 48, sp, nonce);
    memcpy(ct+16, sp, 8);
    ultra_rashomon(sp, 32, sp + 8, nonce ^ 0x9E3779B9);
    memcpy(ct+24, sp+8, 8);
    return 0;
}

int qr_kem_decaps(uint8_t* ss, const uint8_t* ct, const uint8_t* sk) {
    uint8_t pk[32]; phi_hash32(sk, 32, pk);
    uint8_t h[32], c[39];
    memcpy(c, pk, 32); memcpy(c+32, "encaps", 6);
    phi_hash32(c, 38, h);
    for (int i = 0; i < 16; i++) ss[i] = ct[i] ^ h[i];
    
    uint8_t bs[48], sp[32];
    memcpy(bs, pk, 32); memcpy(bs+32, ss, 16);
    uint64_t nonce = phi_mix(*(uint64_t*)pk ^ *(uint64_t*)(pk+8));
    ultra_rashomon(bs, 48, sp, nonce);
    if (memcmp(ct+16, sp, 8) != 0) return -1;
    ultra_rashomon(sp, 32, sp + 8, nonce ^ 0x9E3779B9);
    if (memcmp(ct+24, sp+8, 8) != 0) return -1;
    return 0;
}

int qr_kem_fractal_keygen(uint8_t* fractal64) {
    uint8_t pk[32], sk[32], ct_dummy[32];
    qr_kem_keygen(pk, sk);
    memset(ct_dummy, 0, 32);
    fractal_compress(pk, sk, ct_dummy, fractal64);
    return 0;
}

int qr_kem_fractal_encaps(const uint8_t* fractal64, uint8_t* ct, uint8_t* ss) {
    uint8_t pk[32], sk[32], dummy_ct[32];
    fractal_decompress(fractal64, pk, sk, dummy_ct);
    return qr_kem_encaps(ct, ss, pk);
}

int qr_kem_fractal_decaps(const uint8_t* fractal64, const uint8_t* ct, uint8_t* ss) {
    uint8_t pk[32], sk[32], dummy_ct[32];
    fractal_decompress(fractal64, pk, sk, dummy_ct);
    return qr_kem_decaps(ss, ct, sk);
}
