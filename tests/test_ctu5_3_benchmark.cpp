#include "../src/core/banach_engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace banach;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  CTU v5.3 — 8 DEMON GATES BENCHMARK" << std::endl;
    std::cout << "  8 Engines, 8 Layers" << std::endl;
    std::cout << "  100M Ops, -O0, Every 5M" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    NDimBanachEngine engine;
    
    const long long TOTAL = 100000000LL;
    const long long BATCH = 5000000LL;  // 5M per report!
    long long errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (long long batch = 0; batch < TOTAL / BATCH; batch++) {
        auto batch_start = std::chrono::high_resolution_clock::now();
        
        for (long long i = 0; i < BATCH; i++) {
            long long val = (batch * BATCH + i) % 10000;
            auto ct = engine.encrypt(val, 0);
            int64_t dec = engine.decrypt(ct);
            if (dec != val) errors++;
        }
        
        auto batch_end = std::chrono::high_resolution_clock::now();
        auto batch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start).count();
        long long total_ops = (batch + 1) * BATCH;
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - start).count();
        double tps = total_ops / (total_ms / 1000.0);
        
        std::cout << std::fixed << std::setprecision(1)
                  << "[" << std::setw(2) << (batch + 1) * 5 << "%] "
                  << total_ops / 1000000 << "M | "
                  << batch_ms << "ms | "
                  << (long long)tps << " TPS | "
                  << "Err: " << errors
                  << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  FINAL: " << TOTAL/1000000 << "M ops" << std::endl;
    std::cout << "  Time:  " << total_ms/1000.0 << " seconds" << std::endl;
    std::cout << "  TPS:   " << (long long)(TOTAL/(total_ms/1000.0)) << std::endl;
    std::cout << "  Errors: " << errors << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
