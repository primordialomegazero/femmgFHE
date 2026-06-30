#include "phi_algo_merge.h"
#include <stdio.h>

int main() {
    printf("Test: Deterministic vs Random\n");
    printf("==============================\n\n");
    
    // Same seed -> same chain?
    uint8_t seed1[32] = {0x01, 0x02, 0x03, 0x04};
    uint8_t seed2[32] = {0x01, 0x02, 0x03, 0x04};
    uint8_t chain1[96], chain2[96];
    
    phi_chaotic_chain(seed1, chain1, 6);
    phi_chaotic_chain(seed2, chain2, 6);
    
    int same = 1;
    for(int i=0; i<96; i++) {
        if(chain1[i] != chain2[i]) { same = 0; break; }
    }
    printf("Same seed -> same chain: %s\n", same ? "✅" : "❌");
    
    // Different seed -> different chain
    uint8_t seed3[32] = {0xFF, 0xFE, 0xFD, 0xFC};
    uint8_t chain3[96];
    phi_chaotic_chain(seed3, chain3, 6);
    
    int diff = 0;
    for(int i=0; i<96; i++) {
        if(chain1[i] != chain3[i]) { diff = 1; break; }
    }
    printf("Different seed -> different chain: %s\n", diff ? "✅" : "❌");
    
    // Nonce: same seed -> different nonce (dapat iba!)
    uint8_t nonce1[32], nonce2[32];
    phi_hybrid_nonce(nonce1, 32);
    phi_hybrid_nonce(nonce2, 32);
    
    int nonce_diff = 0;
    for(int i=0; i<32; i++) {
        if(nonce1[i] != nonce2[i]) { nonce_diff = 1; break; }
    }
    printf("Nonce non-deterministic: %s\n", nonce_diff ? "✅" : "❌");
    
    return 0;
}
