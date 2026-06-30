#include "phi_algo_merge.h"
#include <stdio.h>

// Count bits different
int count_bits_diff(const uint8_t *a, const uint8_t *b, size_t len) {
    int diff = 0;
    for(size_t i=0; i<len; i++) {
        uint8_t x = a[i] ^ b[i];
        while(x) { diff += x & 1; x >>= 1; }
    }
    return diff;
}

int main() {
    printf("Test: Avalanche Effect (Chaos Sensitivity)\n");
    printf("===========================================\n\n");
    
    // Seed with single bit difference
    uint8_t seed1[32] = {0};
    uint8_t seed2[32] = {0};
    seed2[0] = 0x01;  // 1 bit difference
    
    uint8_t chain1[96], chain2[96];
    phi_chaotic_chain(seed1, chain1, 6);
    phi_chaotic_chain(seed2, chain2, 6);
    
    int bits_diff = count_bits_diff(chain1, chain2, 96);
    printf("1-bit seed change -> %d bits different in chain\n", bits_diff);
    printf("Avalanche effect: %s\n", bits_diff > 10 ? "✅" : "❌");
    
    // Nonce avalanche
    uint8_t nonce1[32], nonce2[32];
    phi_hybrid_nonce(nonce1, 32);
    phi_hybrid_nonce(nonce2, 32);
    
    int nonce_bits = count_bits_diff(nonce1, nonce2, 32);
    printf("Two nonces: %d bits different\n", nonce_bits);
    printf("Nonce diversity: %s\n\n", nonce_bits > 8 ? "✅" : "❌");
    
    return 0;
}
