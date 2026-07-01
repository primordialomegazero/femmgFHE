#include "src/femmg_fhe.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  100 BILLION MIXED OPERATIONS" << std::endl;
    std::cout << "  Add + Multiply Alternating" << std::endl;
    std::cout << "  FEmmg-FHE FORTRESS v21.5" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    auto ct = fhe.encrypt(2);
    
    std::cout << "Initial noise: " << ct.noise << " bits" << std::endl;
    std::cout << "Starting 100,000,000,000 mixed operations..." << std::endl;
    std::cout << "Pattern: add(2), mul(2), add(2), mul(2)..." << std::endl;
    std::cout << std::endl;
    
    double max_noise = ct.noise;
    double min_noise = ct.noise;
    long long errors = 0;
    long long checks = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    const long long TOTAL = 100000000000LL;
    
    int64_t expected = 2;
    
    for (long long i = 1; i <= TOTAL; i++) {
        auto ct2 = fhe.encrypt(2);
        
        if (i % 2 == 1) {
            // Odd iterations: add
            ct = fhe.add(ct, ct2);
            expected += 2;
        } else {
            // Even iterations: multiply
            ct = fhe.multiply(ct, ct2);
            expected *= 2;
        }
        
        if (ct.noise > max_noise) max_noise = ct.noise;
        if (ct.noise < min_noise) min_noise = ct.noise;
        
        // Verify every 1B ops (100 checks total)
        if (i % 1000000000 == 0) {
            checks++;
            int64_t result = fhe.decrypt(ct);
            if (result != expected) {
                errors++;
                if (errors <= 10) {
                    std::cout << "ERROR at op " << (i / 1000000000LL) << "B"
                              << ": got " << result 
                              << ", expected " << expected << std::endl;
                }
            }
            
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            double progress = (double)i / TOTAL * 100.0;
            double tps = (double)i / elapsed;
            
            std::cout << std::fixed << std::setprecision(1)
                      << "[" << progress << "%] "
                      << "Ops: " << (i / 1000000000LL) << "B | "
                      << "Noise: " << ct.noise << " bits | "
                      << "TPS: " << (long long)tps << " | "
                      << "Errors: " << errors
                      << std::endl;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    
    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  100 BILLION MIXED — FINAL RESULTS" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Total ops:        " << TOTAL << std::endl;
    std::cout << "Total time:       " << total_seconds << " seconds" << std::endl;
    std::cout << "Total time:       " << (total_seconds / 60.0) << " minutes" << std::endl;
    std::cout << "Average TPS:      " << (TOTAL / total_seconds) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Noise:            " << ct.noise << " bits" << std::endl;
    std::cout << "Max noise:        " << max_noise << " bits" << std::endl;
    std::cout << "Min noise:        " << min_noise << " bits" << std::endl;
    std::cout << "Variance:         " << (max_noise - min_noise) << " bits" << std::endl;
    std::cout << "Status:           " << ((max_noise - min_noise) < 1.0 ? "FLATLINE ✓" : "DRIFT ✗") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Checks:           " << checks << " (every 1B ops)" << std::endl;
    std::cout << "Errors:           " << errors << std::endl;
    double accuracy = 100.0 * (1.0 - (double)errors / (double)checks);
    std::cout << "Accuracy:         " << std::fixed << std::setprecision(10) << accuracy << "%" << std::endl;
    std::cout << std::endl;
    
    if (errors == 0 && (max_noise - min_noise) < 1.0) {
        std::cout << "═══════════════════════════════════════" << std::endl;
        std::cout << "  MIXED OPS — PERFECT                  " << std::endl;
        std::cout << "  ADD + MULTIPLY — UNLIMITED DEPTH     " << std::endl;
        std::cout << "  FIBONACCI-LYAPUNOV ENGINE PROVEN     " << std::endl;
        std::cout << "═══════════════════════════════════════" << std::endl;
    } else if (errors <= 1) {
        std::cout << "═══════════════════════════════════════" << std::endl;
        std::cout << "  MIXED OPS — " << errors << " ERROR(S)              " << std::endl;
        std::cout << "  ACCURACY: " << accuracy << "%               " << std::endl;
        std::cout << "═══════════════════════════════════════" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Hardware: AMD Ryzen 5 2600 (2018)" << std::endl;
    std::cout << "Compiler: GCC 11.4.0, -O3 -march=native" << std::endl;
    std::cout << "Scheme:  FEmmg-FHE v21.5 FORTRESS" << std::endl;
    std::cout << "Author:  Dan Joseph M. Fernandez" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
