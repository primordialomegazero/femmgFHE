#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  1 TRILLION OPS — BATCHED (100B × 10)         ║\n";
    std::cout << "║  " << banach::NDimBanachEngine::description() << "\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    const int64_t BATCH = 100000000000LL;  // 100 billion per batch
    const int BATCHES = 10;                 // 10 batches = 1 trillion
    const int64_t TOTAL = BATCH * BATCHES;
    
    auto one_ct = fhe.encrypt(1);
    auto acc = fhe.encrypt(0);
    long double min_noise = acc.noise, max_noise = acc.noise;
    
    auto total_start = std::chrono::high_resolution_clock::now();
    int64_t total_ops = 0;
    
    for(int b = 0; b < BATCHES; b++) {
        std::cout << "Batch " << (b+1) << "/" << BATCHES << " (100B ops)... " << std::flush;
        auto batch_start = std::chrono::high_resolution_clock::now();
        
        // Process 100B adds
        for(int64_t i = 0; i < BATCH; i++) {
            acc = fhe.add(acc, one_ct);
            total_ops++;
        }
        
        auto batch_end = std::chrono::high_resolution_clock::now();
        auto batch_sec = std::chrono::duration_cast<std::chrono::seconds>(batch_end - batch_start).count();
        auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(batch_end - total_start).count();
        
        if(acc.noise < min_noise) min_noise = acc.noise;
        if(acc.noise > max_noise) max_noise = acc.noise;
        
        std::cout << "Done in " << batch_sec << "s | "
                  << "Total: " << (total_ops/1000000000.0) << "B | "
                  << "Noise: " << (double)min_noise << "-" << (double)max_noise << "\n";
    }
    
    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(total_end - total_start).count();
    
    int64_t result = fhe.decrypt(acc);
    
    std::cout << "\n══════════════════════════════════════════════\n";
    std::cout << "  TOTAL: " << total_ops << " operations\n";
    std::cout << "  TIME: " << total_sec << " seconds\n";
    std::cout << "  AVG TPS: " << (total_ops / (total_sec + 1) / 1000000) << "M\n";
    std::cout << "  RESULT: " << result << " (expected " << TOTAL << ")\n";
    std::cout << "  ERROR: " << (TOTAL - result) << "\n";
    std::cout << "  ACCURACY: " << (100.0 - (double)(TOTAL-result)/TOTAL*100.0) << "%\n";
    std::cout << "  NOISE: " << (double)min_noise << " - " << (double)max_noise << "\n";
    std::cout << "  NOISE STABLE: " << ((max_noise-min_noise)<1.0 ? "✅" : "❌") << "\n";
    std::cout << "══════════════════════════════════════════════\n";
    
    return 0;
}
