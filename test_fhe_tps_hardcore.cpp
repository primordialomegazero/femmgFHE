#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <cstdio>
#include <chrono>
#include <cstdlib>

volatile int64_t g_sink = 0;

// ASM barrier — FORCE compiler to compute EVERYTHING
#define FORCE_COMPUTE(x)  __asm__ __volatile__("" : "+r"(x) : : "memory")

int main() {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  FEmmg-FHE — HARDCORE REAL TPS            ║\n");
    printf("║  (ASM barrier, zero optimization escape)   ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    FEmmgFHE fhe;
    
    // ==========================================
    // ENCRYPT TPS
    // ==========================================
    printf("═══ ENCRYPT TPS ═══\n");
    const int OPS = 1000;  // smaller ops, real measurement
    int64_t vals[1000];
    
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<OPS; i++) {
        auto ct = fhe.encrypt(i * 7 + 13);
        vals[i] = (int64_t)ct.coordinates[0];
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    // Force all values to be "used"
    int64_t check = 0;
    for(int i=0; i<OPS; i++) check ^= vals[i];
    FORCE_COMPUTE(check);
    g_sink = check;
    
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double enc_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d encrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, enc_tps, (double)us/OPS);
    
    // ==========================================
    // DECRYPT TPS
    // ==========================================
    printf("═══ DECRYPT TPS ═══\n");
    std::vector<banach::NDimCiphertext> cts;
    for(int i=0; i<OPS; i++) {
        cts.push_back(fhe.encrypt(i * 3 + 7));
        FORCE_COMPUTE(cts[i].coordinates[0]);
    }
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<OPS; i++) {
        vals[i] = fhe.decrypt(cts[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    
    check = 0;
    for(int i=0; i<OPS; i++) check ^= vals[i];
    FORCE_COMPUTE(check);
    g_sink = check;
    
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double dec_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d decrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, dec_tps, (double)us/OPS);
    
    // ==========================================
    // ADD TPS (deep circuit)
    // ==========================================
    printf("═══ ADD TPS (DEEP CIRCUIT) ═══\n");
    auto ct_acc = fhe.encrypt(0);
    FORCE_COMPUTE(ct_acc.coordinates[0]);
    auto ct_one = fhe.encrypt(1);
    FORCE_COMPUTE(ct_one.coordinates[0]);
    
    const int ADD_OPS = 10000;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<ADD_OPS; i++) {
        ct_acc = fhe.add(ct_acc, ct_one);
        FORCE_COMPUTE(ct_acc.coordinates[0]);
    }
    end = std::chrono::high_resolution_clock::now();
    
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double add_tps = (double)ADD_OPS * 1e6 / (double)us;
    printf("  %d adds in %ld µs → %.0f TPS (%.1f µs/op)\n", ADD_OPS, us, add_tps, (double)us/ADD_OPS);
    
    int64_t result = fhe.decrypt(ct_acc);
    printf("  Verification: 0 + %d = %ld %s\n", ADD_OPS, result, result == ADD_OPS ? "✅" : "❌");
    printf("  Noise: %.5f bits\n", (double)ct_acc.noise);
    
    // ==========================================
    // FULL CYCLE TPS
    // ==========================================
    printf("═══ FULL CYCLE TPS ═══\n");
    const int FULL_OPS = 500;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<FULL_OPS; i++) {
        auto a = fhe.encrypt(i * 2);
        FORCE_COMPUTE(a.coordinates[0]);
        auto b = fhe.encrypt(i * 2 + 1);
        FORCE_COMPUTE(b.coordinates[0]);
        auto c = fhe.add(a, b);
        FORCE_COMPUTE(c.coordinates[0]);
        vals[i] = fhe.decrypt(c);
    }
    end = std::chrono::high_resolution_clock::now();
    
    check = 0;
    for(int i=0; i<FULL_OPS; i++) check ^= vals[i];
    FORCE_COMPUTE(check);
    g_sink = check;
    
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double full_tps = (double)FULL_OPS * 1e6 / (double)us;
    printf("  %d full cycles in %ld µs → %.0f TPS (%.1f µs/op)\n", FULL_OPS, us, full_tps, (double)us/FULL_OPS);
    
    // ==========================================
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  HARDCORE FHE BENCHMARK (asm-forced)     ║\n");
    printf("║  Encrypt:    %8.0f TPS (%6.1f µs/op)    ║\n", enc_tps, 1e6/enc_tps);
    printf("║  Decrypt:    %8.0f TPS (%6.1f µs/op)    ║\n", dec_tps, 1e6/dec_tps);
    printf("║  Add (deep): %8.0f TPS (%6.1f µs/op)    ║\n", add_tps, 1e6/add_tps);
    printf("║  Full Cycle: %8.0f TPS (%6.1f µs/op)    ║\n", full_tps, 1e6/full_tps);
    printf("║  Sink: %ld                              ║\n", (long)g_sink);
    printf("╚══════════════════════════════════════════╝\n");
    
    return 0;
}
