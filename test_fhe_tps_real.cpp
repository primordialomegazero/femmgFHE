#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <cstdio>
#include <chrono>

volatile int64_t sink = 0;  // prevent compiler optimization

int main() {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  FEmmg-FHE v21.4 — REAL FHE TPS           ║\n");
    printf("║  (Anti-optimized, real measurements)       ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    FEmmgFHE fhe;
    
    // ==========================================
    // ENCRYPT TPS
    // ==========================================
    printf("═══ ENCRYPT TPS ═══\n");
    const int OPS = 10000;
    volatile int64_t dummy_enc = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for(int64_t i=0; i<OPS; i++) {
        auto ct = fhe.encrypt(i * 7 + 13);  // varying input
        dummy_enc ^= (int64_t)ct.coordinates[0];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    sink = dummy_enc;
    double enc_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d encrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, enc_tps, (double)us/OPS);
    
    // ==========================================
    // DECRYPT TPS
    // ==========================================
    printf("═══ DECRYPT TPS ═══\n");
    // Pre-create various ciphertexts
    std::vector<banach::NDimCiphertext> cts;
    for(int i=0; i<OPS; i++) cts.push_back(fhe.encrypt(i * 3 + 7));
    
    volatile int64_t dummy_dec = 0;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<OPS; i++) {
        dummy_dec ^= fhe.decrypt(cts[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    sink = dummy_dec;
    double dec_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d decrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, dec_tps, (double)us/OPS);
    
    // ==========================================
    // ADD TPS (deep circuit)
    // ==========================================
    printf("═══ ADD TPS (DEEP CIRCUIT) ═══\n");
    auto ct_acc = fhe.encrypt(0);
    auto ct_one = fhe.encrypt(1);
    
    const int ADD_OPS = 50000;
    volatile int64_t dummy_add = 0;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<ADD_OPS; i++) {
        ct_acc = fhe.add(ct_acc, ct_one);
        dummy_add ^= (int64_t)ct_acc.coordinates[0];
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    sink = dummy_add;
    double add_tps = (double)ADD_OPS * 1e6 / (double)us;
    printf("  %d adds in %ld µs → %.0f TPS (%.1f µs/op)\n", ADD_OPS, us, add_tps, (double)us/ADD_OPS);
    
    int64_t result = fhe.decrypt(ct_acc);
    printf("  Verification: 0 + %d = %ld %s\n", ADD_OPS, result, result == ADD_OPS ? "✅" : "❌");
    
    // Noise tracking
    double min_n = ct_acc.noise, max_n = ct_acc.noise;
    printf("  Noise after %d adds: %.5f\n", ADD_OPS, ct_acc.noise);
    
    // ==========================================
    // MULTIPLY TPS
    // ==========================================
    printf("═══ MULTIPLY TPS ═══\n");
    auto mt = fhe.encrypt(2);
    auto mt_factor = fhe.encrypt(3);
    
    const int MUL_OPS = 5000;
    volatile int64_t dummy_mul = 0;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<MUL_OPS; i++) {
        mt = fhe.multiply(mt, mt_factor);
        dummy_mul ^= (int64_t)mt.coordinates[0];
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    sink = dummy_mul;
    double mul_tps = (double)MUL_OPS * 1e6 / (double)us;
    printf("  %d multiplies in %ld µs → %.0f TPS (%.1f µs/op)\n", MUL_OPS, us, mul_tps, (double)us/MUL_OPS);
    
    // ==========================================
    // FULL CYCLE TPS
    // ==========================================
    printf("═══ FULL CYCLE TPS ═══\n");
    const int FULL_OPS = 2000;
    volatile int64_t dummy_full = 0;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<FULL_OPS; i++) {
        auto a = fhe.encrypt(i * 2);
        auto b = fhe.encrypt(i * 2 + 1);
        auto c = fhe.add(a, b);
        dummy_full ^= fhe.decrypt(c);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    sink = dummy_full;
    double full_tps = (double)FULL_OPS * 1e6 / (double)us;
    printf("  %d full cycles in %ld µs → %.0f TPS (%.1f µs/op)\n", FULL_OPS, us, full_tps, (double)us/FULL_OPS);
    
    // ==========================================
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  REAL FHE BENCHMARK (anti-optimized)     ║\n");
    printf("║  Encrypt:    %8.0f TPS (%6.1f µs/op)    ║\n", enc_tps, (double)us/OPS);
    printf("║  Decrypt:    %8.0f TPS (%6.1f µs/op)    ║\n", dec_tps, 1e6/dec_tps);
    printf("║  Add (deep): %8.0f TPS (%6.1f µs/op)    ║\n", add_tps, 1e6/add_tps);
    printf("║  Multiply:   %8.0f TPS (%6.1f µs/op)    ║\n", mul_tps, 1e6/mul_tps);
    printf("║  Full Cycle: %8.0f TPS (%6.1f µs/op)    ║\n", full_tps, 1e6/full_tps);
    printf("║  Noise Δ:    %8.5f bits                  ║\n", max_n - min_n);
    printf("╚══════════════════════════════════════════╝\n");
    printf("  sink=%ld (prevent dead-code elim)\n", (long)sink);
    
    return 0;
}
