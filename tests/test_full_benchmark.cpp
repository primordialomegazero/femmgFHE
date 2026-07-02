/*
 * FEmmg-FHE v22.2 — Full Benchmark Suite
 *
 * Metrics:
 *   - Throughput (TPS)
 *   - Latency (p50, p95, p99)
 *   - Memory usage (ciphertext size)
 *   - Avalanche effect (bits flipped)
 *   - Noise stability (over 10K operations)
 */

#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <numeric>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.2 — FULL BENCHMARK SUITE" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    // ═══ 1. THROUGHPUT (TPS) ═══
    std::cout << "\n═══ THROUGHPUT ═══" << std::endl;
    const int WARMUP = 1000;
    const int ITERS = 10000;
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        auto a = fhe.encrypt(42);
        volatile auto d __attribute__((unused)) = fhe.decrypt(a);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto a = fhe.encrypt(i);
        volatile auto d __attribute__((unused)) = fhe.decrypt(a);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double tps = ITERS * 1e6 / dur_us;
    std::cout << "  Encrypt+Decrypt TPS: " << std::fixed << std::setprecision(0) << tps << " ops/sec" << std::endl;
    std::cout << "  Avg latency: " << std::fixed << std::setprecision(2) << (dur_us / (double)ITERS) << " µs/op" << std::endl;

    // ═══ 2. LATENCY DISTRIBUTION ═══
    std::cout << "\n═══ LATENCY DISTRIBUTION ═══" << std::endl;
    const int LAT_ITERS = 1000;
    std::vector<double> latencies;
    latencies.reserve(LAT_ITERS);
    
    for (int i = 0; i < LAT_ITERS; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        auto a = fhe.encrypt(42);
        volatile auto d __attribute__((unused)) = fhe.decrypt(a);
        auto t2 = std::chrono::high_resolution_clock::now();
        latencies.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
    }
    
    std::sort(latencies.begin(), latencies.end());
    double p50 = latencies[LAT_ITERS * 50 / 100];
    double p95 = latencies[LAT_ITERS * 95 / 100];
    double p99 = latencies[LAT_ITERS * 99 / 100];
    double avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / LAT_ITERS;
    double min_lat = latencies.front();
    double max_lat = latencies.back();
    
    std::cout << "  Samples: " << LAT_ITERS << std::endl;
    std::cout << "  Min:  " << std::fixed << std::setprecision(1) << min_lat << " µs" << std::endl;
    std::cout << "  Avg:  " << avg << " µs" << std::endl;
    std::cout << "  P50:  " << p50 << " µs" << std::endl;
    std::cout << "  P95:  " << p95 << " µs" << std::endl;
    std::cout << "  P99:  " << p99 << " µs" << std::endl;
    std::cout << "  Max:  " << max_lat << " µs" << std::endl;

    // ═══ 3. MEMORY / CIPHERTEXT SIZE ═══
    std::cout << "\n═══ CIPHERTEXT SIZE ═══" << std::endl;
    auto ct = fhe.encrypt(42);
    size_t ct_size = sizeof(ct);
    std::cout << "  NDimCiphertext: " << ct_size << " bytes" << std::endl;
    std::cout << "  value_int: " << sizeof(ct.value_int) << " bytes" << std::endl;
    std::cout << "  coordinates[7]: " << sizeof(ct.coordinates) << " bytes" << std::endl;
    std::cout << "  chaos_history[21]: " << sizeof(ct.chaos_history) << " bytes" << std::endl;
    std::cout << "  Integrity tag: " << sizeof(ct.integrity_tag) << " bytes" << std::endl;
    std::cout << "  Random IV: " << sizeof(ct.random_iv) << " bytes" << std::endl;

    // ═══ 4. AVALANCHE EFFECT ═══
    std::cout << "\n═══ AVALANCHE (42 vs 43) ═══" << std::endl;
    auto ct42 = fhe.encrypt(42);
    auto ct43 = fhe.encrypt(43);
    
    auto count_bits = [](const void* a, const void* b, size_t sz) -> int {
        int bits = 0;
        const uint8_t* pa = static_cast<const uint8_t*>(a);
        const uint8_t* pb = static_cast<const uint8_t*>(b);
        for (size_t i = 0; i < sz; i++) {
            uint8_t diff = pa[i] ^ pb[i];
            while (diff) { bits += diff & 1; diff >>= 1; }
        }
        return bits;
    };
    
    int coord_bits = count_bits(ct42.coordinates.data(), ct43.coordinates.data(), sizeof(ct42.coordinates));
    int chaos_bits = count_bits(ct42.chaos_history, ct43.chaos_history, sizeof(ct42.chaos_history));
    int tag_bits = count_bits(&ct42.integrity_tag, &ct43.integrity_tag, sizeof(ct42.integrity_tag));
    int total_bits = count_bits(&ct42, &ct43, sizeof(ct42));
    
    std::cout << "  Coordinates diff: " << coord_bits << " bits" << std::endl;
    std::cout << "  Chaos history diff: " << chaos_bits << " bits" << std::endl;
    std::cout << "  Tag diff: " << tag_bits << " bits" << std::endl;
    std::cout << "  Total ciphertext diff: " << total_bits << " bits (of " << (sizeof(ct42)*8) << ")" << std::endl;
    std::cout << "  Avalanche %: " << std::fixed << std::setprecision(1) 
              << (100.0 * total_bits / (sizeof(ct42)*8)) << "%" << std::endl;

    // ═══ 5. NOISE STABILITY ═══
    std::cout << "\n═══ NOISE STABILITY (10K ops) ═══" << std::endl;
    auto ct_noise = fhe.encrypt(1);
    double min_noise = ct_noise.noise;
    double max_noise = ct_noise.noise;
    std::vector<double> noise_samples;
    noise_samples.reserve(10000);
    
    for (int i = 0; i < 10000; i++) {
        auto ct_one = fhe.encrypt(1);
        ct_noise = fhe.add(ct_noise, ct_one);
        noise_samples.push_back(ct_noise.noise);
        if (ct_noise.noise < min_noise) min_noise = ct_noise.noise;
        if (ct_noise.noise > max_noise) max_noise = ct_noise.noise;
    }
    
    double avg_noise = std::accumulate(noise_samples.begin(), noise_samples.end(), 0.0) / noise_samples.size();
    double noise_range = max_noise - min_noise;
    
    std::cout << "  Min noise:  " << std::fixed << std::setprecision(6) << min_noise << " bits" << std::endl;
    std::cout << "  Max noise:  " << max_noise << " bits" << std::endl;
    std::cout << "  Avg noise:  " << avg_noise << " bits" << std::endl;
    std::cout << "  Range:      " << noise_range << " bits" << std::endl;
    
    // Verify correctness
    int64_t final_val = fhe.decrypt(ct_noise);
    std::cout << "  Final value: " << final_val << " (expected " << (1 + 10000) << ")" 
              << (final_val == 10001 ? " ✅" : " ❌") << std::endl;
    
    if (noise_range < 1.0) {
        std::cout << "  NOISE FLATLINE ✅ (range < 1 bit over 10K ops)" << std::endl;
    } else {
        std::cout << "  ⚠️ Noise growing (range = " << noise_range << " bits)" << std::endl;
    }

    // ═══ 6. HOMOMORPHIC THROUGHPUT ═══
    std::cout << "\n═══ HOMOMORPHIC THROUGHPUT ═══" << std::endl;
    const int HOM_ITERS = 5000;
    
    // Addition throughput
    auto hstart = std::chrono::high_resolution_clock::now();
    auto ct_acc = fhe.encrypt(0);
    for (int i = 0; i < HOM_ITERS; i++) {
        auto ct_one = fhe.encrypt(1);
        ct_acc = fhe.add(ct_acc, ct_one);
    }
    auto hend = std::chrono::high_resolution_clock::now();
    auto hom_add_us = std::chrono::duration_cast<std::chrono::microseconds>(hend - hstart).count();
    double add_tps = HOM_ITERS * 1e6 / hom_add_us;
    int64_t add_result = fhe.decrypt(ct_acc);
    
    std::cout << "  Add TPS: " << std::fixed << std::setprecision(0) << add_tps << " ops/sec" << std::endl;
    std::cout << "  Add latency: " << std::fixed << std::setprecision(2) << (hom_add_us / (double)HOM_ITERS) << " µs/op" << std::endl;
    std::cout << "  Result: " << add_result << " (expected " << HOM_ITERS << ")" 
              << (add_result == HOM_ITERS ? " ✅" : " ❌") << std::endl;

    // Multiplication throughput
    auto mstart = std::chrono::high_resolution_clock::now();
    auto ct_mul_acc = fhe.encrypt(2);
    for (int i = 0; i < 10; i++) {  // Fewer mults to avoid overflow
        auto ct_two = fhe.encrypt(2);
        ct_mul_acc = fhe.multiply(ct_mul_acc, ct_two);
    }
    auto mend = std::chrono::high_resolution_clock::now();
    auto hom_mul_us = std::chrono::duration_cast<std::chrono::microseconds>(mend - mstart).count();
    double mul_tps = 10 * 1e6 / hom_mul_us;
    
    std::cout << "  Mul TPS: " << std::fixed << std::setprecision(0) << mul_tps << " ops/sec" << std::endl;
    std::cout << "  Mul latency: " << std::fixed << std::setprecision(2) << (hom_mul_us / 10.0) << " µs/op" << std::endl;

    // ═══ SUMMARY ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  BENCHMARK SUMMARY" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  Encrypt+Decrypt TPS: " << std::setprecision(0) << tps << std::endl;
    std::cout << "  Avg Latency: " << std::setprecision(2) << avg << " µs (P50: " << p50 << ", P99: " << p99 << ")" << std::endl;
    std::cout << "  Ciphertext Size: " << ct_size << " bytes" << std::endl;
    std::cout << "  Avalanche: " << std::setprecision(1) << (100.0 * total_bits / (sizeof(ct42)*8)) << "%" << std::endl;
    std::cout << "  Noise Stability: " << std::setprecision(6) << noise_range << " bits range over 10K ops" << std::endl;
    std::cout << "  Homomorphic Add TPS: " << std::setprecision(0) << add_tps << std::endl;
    std::cout << "  Homomorphic Mul TPS: " << mul_tps << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    return 0;
}
