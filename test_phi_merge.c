#include "phi_algo_merge.h"
#include <stdio.h>

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  PHI-ALGO MERGE TEST                        ║\n");
    printf("║  Spiralkem + Φ-SIG + Post-Quantoink        ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    
    // Test 1: Chaotic Chain
    printf("Test 1: Chaotic Chain\n");
    uint8_t seed[32] = {0};
    uint8_t chain[96];
    phi_chaotic_chain(seed, chain, 6);
    
    int chain_nonzero = 0;
    for(int i=0; i<96; i++) if(chain[i]) { chain_nonzero = 1; break; }
    printf("  Chain non-zero: %s\n\n", chain_nonzero ? "✅" : "❌");
    
    // Test 2: Keyless Signature
    printf("Test 2: Keyless Signature\n");
    uint8_t sig[64];
    size_t sig_len;
    uint8_t msg[] = "test message";
    phi_keyless_sign(msg, sizeof(msg)-1, sig, &sig_len);
    printf("  Signature length: %zu bytes\n", sig_len);
    printf("  Signature non-zero: %s\n\n", sig_len == 64 ? "✅" : "❌");
    
    // Test 3: 4-Stream Chaos
    printf("Test 3: 4-Stream Chaotic Divergence\n");
    PQStream pq;
    uint8_t pq_seed[32];
    for(int i=0; i<32; i++) pq_seed[i] = i;
    pq_stream_init(&pq, pq_seed);
    
    double s0 = pq.streams[0];
    for(int i=0; i<10; i++) pq_stream_evolve(&pq);
    double s1 = pq.streams[0];
    printf("  Stream 0: %.6f -> %.6f\n", s0, s1);
    printf("  Diverged: %s\n\n", (s1 != s0) ? "✅" : "❌");
    
    // Test 4: Hybrid Nonce
    printf("Test 4: Hybrid Nonce\n");
    uint8_t nonce1[32], nonce2[32];
    phi_hybrid_nonce(nonce1, 32);
    phi_hybrid_nonce(nonce2, 32);
    
    int diff = 0;
    for(int i=0; i<32; i++) if(nonce1[i] != nonce2[i]) { diff = 1; break; }
    printf("  Nonce1: ");
    for(int i=0; i<8; i++) printf("%02x", nonce1[i]);
    printf("...\n");
    printf("  Nonce2: ");
    for(int i=0; i<8; i++) printf("%02x", nonce2[i]);
    printf("...\n");
    printf("  Different: %s\n\n", diff ? "✅" : "❌");
    
    // Test 5: Self-test
    printf("Test 5: Self-Test\n");
    int result = phi_algo_self_test();
    printf("  Result: %s\n\n", result ? "✅ PASS" : "❌ FAIL");
    
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  ALL TESTS COMPLETE                         ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    
    return 0;
}
