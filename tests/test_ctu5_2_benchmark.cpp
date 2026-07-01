#include "../src/core/banach_engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace banach;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  CTU v5.2 — QUINTUPLE RASHOMON BENCHMARK" << std::endl;
    std::cout << "  5 Engines × 5 Passes = 25 Layers" << std::endl;
    std::cout << "  100M Encrypt+Decrypt Operations" << std::endl;
    std::cout << "  -O0 True Performance" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    NDimBanachEngine engine;
    
    const long long TOTAL = 100000000LL;
    const long long BATCH = 10000000LL;
    long long errors = 0;
    double max_noise = 1.82815;
    double min_noise = 1.82815;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (long long batch = 0; batch < TOTAL / BATCH; batch++) {
        auto batch_start = std::chrono::high_resolution_clock::now();
        
        for (long long i = 0; i < BATCH; i++) {
            long long val = (batch * BATCH + i) % 10000;
            auto ct = engine.encrypt(val, 0);
            int64_t dec = engine.decrypt(ct);
            
            if (dec != val) errors++;
            if (ct.noise > max_noise) max_noise = ct.noise;
            if (ct.noise < min_noise) min_noise = ct.noise;
        }
        
        auto batch_end = std::chrono::high_resolution_clock::now();
        auto batch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start).count();
        long long total_ops = (batch + 1) * BATCH;
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - start).count();
        double tps = total_ops / (total_ms / 1000.0);
        
        std::cout << std::fixed << std::setprecision(1)
                  << "[" << std::setw(3) << (batch + 1) * 10 << "%] "
                  << "Ops: " << total_ops / 1000000 << "M | "
                  << "Batch: " << batch_ms << "ms | "
                  << "TPS: " << (long long)tps << " | "
                  << "Errors: " << errors
                  << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double total_sec = total_ms / 1000.0;
    
    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  FINAL RESULTS" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Total ops:     " << TOTAL / 1000000 << "M" << std::endl;
    std::cout << "  Total time:    " << total_sec << " seconds" << std::endl;
    std::cout << "  Average TPS:   " << (long long)(TOTAL / total_sec) << std::endl;
    std::cout << "  Errors:        " << errors << std::endl;
    std::cout << "  Noise max:     " << max_noise << " bits" << std::endl;
    std::cout << "  Noise min:     " << min_noise << " bits" << std::endl;
    std::cout << "  Noise var:     " << (max_noise - min_noise) << " bits" << std::endl;
    std::cout << "  Accuracy:      " << (100.0 - 100.0 * errors / TOTAL) << "%" << std::endl;
    std::cout << std::endl;
    std::cout << "  🎉 CTU v5.2 — Quintuple Rashomon — Benchmark Complete!" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
