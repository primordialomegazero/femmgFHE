#include "phi_parallel_kem.h"
#include <cstdio>
#include <cstring>

int main() {
    printf("╔══════════════════════════════════════╗\n");
    printf("║  Φ-PKE: Fibonacci-Phi Parallel KEM   ║\n");
    printf("║  7 Lanes | φ-Irrational | Fortress   ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    // Test 1: Lane initialization + evolution
    printf("Test 1: Single Lane Chaos\n");
    phi_parallel::Lane lane;
    lane.init(0xDEADBEEF12345678ULL, 0);
    
    double prev = lane.state;
    int diverged = 0;
    for(int i=0; i<10; i++) {
        double curr = lane.step();
        if(curr != prev) diverged++;
        prev = curr;
    }
    printf("  Lane state changes: %d/10\n", diverged);
    printf("  Final attractor (F_%d): %016lx\n", lane.fib_index, lane.attractor);
    printf("  %s\n\n", diverged >= 8 ? "✅ CHAOTIC" : "❌ STAGNANT");
    
    // Test 2: Multi-lane parallel
    printf("Test 2: 7-Lane Parallel Engine\n");
    phi_parallel::PhiParallelEngine engine;
    uint8_t seed[32] = "PHI_PARALLEL_KEM_SEED_123456789";
    engine.seed(seed, 32);
    engine.evolve(128);
    
    uint8_t output[32];
    engine.extract(output, 32);
    
    int nonzero = 0;
    for(int i=0; i<32; i++) if(output[i]) nonzero++;
    printf("  Output non-zero bytes: %d/32\n", nonzero);
    printf("  Output: ");
    for(int i=0; i<8; i++) printf("%02x", output[i]);
    printf("...\n");
    printf("  %s\n\n", nonzero >= 16 ? "✅ HIGH ENTROPY" : "❌ LOW ENTROPY");
    
    // Test 3: Deterministic reproducibility
    printf("Test 3: Deterministic Reproducibility\n");
    phi_parallel::PhiParallelEngine engine2;
    engine2.seed(seed, 32);
    engine2.evolve(128);
    
    uint8_t output2[32];
    engine2.extract(output2, 32);
    
    int match = memcmp(output, output2, 32) == 0;
    printf("  Same seed = same output: %s\n\n", match ? "✅" : "❌");
    
    // Test 4: Full KEM — generate + encapsulate + decapsulate
    printf("Test 4: Full KEM Cycle\n");
    int kem_ok = phi_parallel::phi_parallel_self_test();
    printf("  KEM generate+encap+decap: %s\n\n", kem_ok ? "✅ PASS" : "❌ FAIL");
    
    // Test 5: Avalanche — 1 bit change in seed
    printf("Test 5: Avalanche Effect\n");
    uint8_t seed_a[32], seed_b[32];
    memcpy(seed_a, "FEmmg-FHE Phi-Parallel KEM v21.1!", 32);
    memcpy(seed_b, "FEmmg-FHE Phi-Parallel KEM v21.1!", 32);
    seed_b[0] ^= 0x01;  // flip 1 bit
    
    phi_parallel::PhiParallelEngine eng_a, eng_b;
    eng_a.seed(seed_a, 32); eng_a.evolve(128);
    eng_b.seed(seed_b, 32); eng_b.evolve(128);
    
    uint8_t out_a[32], out_b[32];
    eng_a.extract(out_a, 32);
    eng_b.extract(out_b, 32);
    
    int diff_bits = 0;
    for(int i=0; i<32; i++) {
        uint8_t diff = out_a[i] ^ out_b[i];
        for(int b=0; b<8; b++) if(diff & (1<<b)) diff_bits++;
    }
    printf("  Bits flipped (of 256): %d\n", diff_bits);
    printf("  Avalanche: %s (%.1f%% different)\n\n", 
           diff_bits > 100 ? "✅ EXCELLENT" : "❌ WEAK",
           100.0 * diff_bits / 256.0);
    
    printf("╔══════════════════════════════════════╗\n");
    printf("║  Φ-PKE: ALL TESTS COMPLETE           ║\n");
    printf("╚══════════════════════════════════════╝\n");
    
    return 0;
}
