#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  1 TRILLION OPERATION STRESS TEST             ║\n";
    std::cout << "║  " << banach::NDimBanachEngine::description() << "\n";
    std::cout << "║  Precision: long double (80-bit)              ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    const int64_t TARGET = 1000000000000LL; // 1 trillion
    
    std::cout << "Running " << TARGET << " homomorphic additions...\n";
    std::cout << "This will take a while. Watch the progress:\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto one_ct = fhe.encrypt(1);
    auto acc = fhe.encrypt(0);
    long double min_noise = acc.noise;
    long double max_noise = acc.noise;
    
    int64_t report_interval = TARGET / 20;
    
    for(int64_t i = 1; i <= TARGET; i++) {
        acc = fhe.add(acc, one_ct);
        
        if(acc.noise < min_noise) min_noise = acc.noise;
        if(acc.noise > max_noise) max_noise = acc.noise;
        
        if(i % report_interval == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            double pct = (double)i / TARGET * 100.0;
            double tps = (double)i / (elapsed + 1);
            
            std::cout << "  " << (int)pct << "% | " << (i/1000000000.0) << "B ops | " 
                      << elapsed << "s | " << (int)(tps/1000000) << "M TPS | "
                      << "noise: " << (double)min_noise << "-" << (double)max_noise << "\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    
    int64_t result = fhe.decrypt(acc);
    
    std::cout << "\n══════════════════════════════════════════════\n";
    std::cout << "  TOTAL: " << TARGET << " operations\n";
    std::cout << "  TIME: " << total_sec << " seconds\n";
    std::cout << "  AVG TPS: " << (TARGET / (total_sec + 1)) << "\n";
    std::cout << "  RESULT: " << result << " (expected " << TARGET << ")\n";
    std::cout << "  CORRECT: " << (result == TARGET ? "✅ YES!" : "❌ NO") << "\n";
    std::cout << "  ERROR: " << (TARGET - result) << " (out of " << TARGET << ")\n";
    std::cout << "  ACCURACY: " << (100.0 - (double)(TARGET - result)/TARGET*100.0) << "%\n";
    std::cout << "  NOISE RANGE: " << (double)min_noise << " - " << (double)max_noise << "\n";
    std::cout << "  NOISE STABLE: " << ((max_noise - min_noise) < 1.0 ? "✅ YES" : "❌ NO") << "\n";
    std::cout << "══════════════════════════════════════════════\n";
    
    return (result == TARGET) ? 0 : 1;
}
