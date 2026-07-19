#include "crypto_backend.h"
#include <string.h>
#include <math.h>
#define PHI 1.6180339887498948482
#define PHI_INV 0.6180339887498948482

static int phi_hash(const unsigned char *in, size_t len, unsigned char *out) {
    double state = PHI;  // Start with φ, not zero!
    
    // Mix input
    for (size_t i = 0; i < len; i++) {
        state = state * PHI + in[i] + 1.0;  // Add 1 to avoid zero
        state = state - floor(state);
    }
    
    // Generate output
    for (size_t i = 0; i < 32; i++) {
        state = state * PHI + (double)(i + 1) + PHI_INV;
        state = state - floor(state);
        out[i] = (unsigned char)(state * 255.0) + 1;  // +1 para never zero!
    }
    return 0;
}

static int phi_rand(unsigned char *buf, size_t len) {
    static double seed = PHI;
    for (size_t i = 0; i < len; i++) {
        seed = seed * PHI + (double)(i + 1) / 255.0 + 1.0;
        seed = seed - floor(seed);
        buf[i] = (unsigned char)(seed * 255.0) + 1;
    }
    return 0;
}

CryptoBackend phi_backend = {
    .sha256 = phi_hash,
    .rand_bytes = phi_rand
};
