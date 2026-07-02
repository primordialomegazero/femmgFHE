#include "../src/core/banach_engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace banach;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  CTU v5 — TRIPLE RASHOMON BENCHMARK" << std::endl;
    std::cout << "  1M Encrypt+Decrypt Operations" << std::endl;
    std::cout << "  -O0 True Performance" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    NDimBanachEngine engine;
    
    const long long TOTAL = 1000000LL;  // 1M!
    const long long BATCH = 100000LL;    // Every 100K
    long long errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (long long batch = 0; batch < TOTAL / BATCH; batch++) {
        auto batch_start = std::chrono::high_resolution_clock::now();
        
        for (long long i = 0; i < BATCH; i++) {
            long long val = (batch * BATCH + i) % 10000;
            auto ct = engine.encrypt(val, 0);
            if (engine.decrypt(ct) != val) errors++;
        }
        
        auto batch_end = std::chrono::high_resolution_clock::now();
        auto batch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start).count();
        long long total_ops = (batch + 1) * BATCH;
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - start).count();
        double tps = total_ops / (total_ms / 1000.0);
        
        std::cout << std::fixed << std::setprecision(1)
                  << "[" << std::setw(3) << (batch + 1) * 10 << "%] "
                  << total_ops / 1000 << "K ops | "
                  << batch_ms << "ms | "
                  << (long long)tps << " TPS | "
                  << "Errors: " << errors
                  << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  FINAL RESULTS" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Total ops:     1,000,000" << std::endl;
    std::cout << "  Total time:    " << total_ms / 1000.0 << " seconds" << std::endl;
    std::cout << "  Average TPS:   " << (long long)(TOTAL / (total_ms / 1000.0)) << std::endl;
    std::cout << "  Errors:        " << errors << std::endl;
    std::cout << "  Accuracy:      " << (100.0 - 100.0 * errors / TOTAL) << "%" << std::endl;
    std::cout << std::endl;
    std::cout << "  🎉 CTU v5 — Triple Rashomon — 1M Ops Complete!" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
