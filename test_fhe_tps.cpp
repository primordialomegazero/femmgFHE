#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <cstdio>
#include <chrono>

int main() {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  FEmmg-FHE v21.4 — FHE TPS BENCHMARK      ║\n");
    printf("║  Encrypt/Add/Multiply/Decrypt              ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    FEmmgFHE fhe;
    
    // ==========================================
    // BENCH 1: ENCRYPT TPS
    // ==========================================
    printf("═══ ENCRYPT TPS ═══\n");
    const int OPS = 50000;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<OPS; i++) {
        auto ct = fhe.encrypt(42);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double enc_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d encrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, enc_tps, (double)us/OPS);
    
    // ==========================================
    // BENCH 2: DECRYPT TPS
    // ==========================================
    printf("═══ DECRYPT TPS ═══\n");
    auto ct_sample = fhe.encrypt(42);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<OPS; i++) {
        int64_t m = fhe.decrypt(ct_sample);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double dec_tps = (double)OPS * 1e6 / (double)us;
    printf("  %d decrypts in %ld µs → %.0f TPS (%.1f µs/op)\n", OPS, us, dec_tps, (double)us/OPS);
    
    // ==========================================
    // BENCH 3: ADD TPS (deep circuit — single ciphertext)
    // ==========================================
    printf("═══ ADD TPS (DEEP CIRCUIT) ═══\n");
    auto ct1 = fhe.encrypt(1);
    auto ct2 = fhe.encrypt(1);
    
    // Warmup
    for(int i=0; i<1000; i++) { ct1 = fhe.add(ct1, ct2); }
    
    const int ADD_OPS = 100000;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<ADD_OPS; i++) {
        ct1 = fhe.add(ct1, ct2);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double add_tps = (double)ADD_OPS * 1e6 / (double)us;
    printf("  %d adds in %ld µs → %.0f TPS (%.1f µs/op)\n", ADD_OPS, us, add_tps, (double)us/ADD_OPS);
    
    // Verify
    int64_t result = fhe.decrypt(ct1);
    printf("  Verification: 1 + %d = %ld %s\n", ADD_OPS+1000, result, result == ADD_OPS+1001 ? "✅" : "❌");
    
    // ==========================================
    // BENCH 4: MULTIPLY TPS
    // ==========================================
    printf("═══ MULTIPLY TPS ═══\n");
    auto mt1 = fhe.encrypt(2);
    auto mt2 = fhe.encrypt(3);
    
    // Warmup
    for(int i=0; i<100; i++) { mt1 = fhe.multiply(mt1, mt2); }
    
    const int MUL_OPS = 10000;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<MUL_OPS; i++) {
        mt1 = fhe.multiply(mt1, mt2);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double mul_tps = (double)MUL_OPS * 1e6 / (double)us;
    printf("  %d multiplies in %ld µs → %.0f TPS (%.1f µs/op)\n", MUL_OPS, us, mul_tps, (double)us/MUL_OPS);
    
    // ==========================================
    // BENCH 5: FULL CYCLE (encrypt+add+decrypt)
    // ==========================================
    printf("═══ FULL CYCLE TPS ═══\n");
    const int FULL_OPS = 5000;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<FULL_OPS; i++) {
        auto a = fhe.encrypt(i);
        auto b = fhe.encrypt(i+1);
        auto c = fhe.add(a, b);
        int64_t m = fhe.decrypt(c);
    }
    end = std::chrono::high_resolution_clock::now();
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double full_tps = (double)FULL_OPS * 1e6 / (double)us;
    printf("  %d full cycles in %ld µs → %.0f TPS (%.1f µs/op)\n", FULL_OPS, us, full_tps, (double)us/FULL_OPS);
    
    // ==========================================
    // BENCH 6: NOISE STABILITY CHECK
    // ==========================================
    printf("═══ NOISE CHECK ═══\n");
    auto noise_ct = fhe.encrypt(0);
    double min_n = noise_ct.noise, max_n = noise_ct.noise;
    for(int i=0; i<10000; i++) {
        noise_ct = fhe.add(noise_ct, ct2);
        if(noise_ct.noise < min_n) min_n = noise_ct.noise;
        if(noise_ct.noise > max_n) max_n = noise_ct.noise;
    }
    printf("  Noise after 10K adds: [%.5f, %.5f]\n", min_n, max_n);
    printf("  Deviation: %.10f %s\n\n", max_n - min_n, (max_n-min_n)<0.01 ? "✅ STABLE" : "❌ DRIFT");
    
    // ==========================================
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  FHE BENCHMARK SUMMARY                   ║\n");
    printf("║  Encrypt:    %.0f TPS                    ║\n", enc_tps);
    printf("║  Decrypt:    %.0f TPS                    ║\n", dec_tps);
    printf("║  Add (deep): %.0f TPS                    ║\n", add_tps);
    printf("║  Multiply:   %.0f TPS                    ║\n", mul_tps);
    printf("║  Full Cycle: %.0f TPS                    ║\n", full_tps);
    printf("╚══════════════════════════════════════════╝\n");
    
    return 0;
}
