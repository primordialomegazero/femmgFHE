#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  10 BILLION OPERATIONS — TRUE DEEP CIRCUIT    ║\n";
    std::cout << "║  Fibonacci-Lyapunov + long double (80-bit)    ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    const int64_t TOTAL = 10000000000LL; // 10 billion
    const int64_t REPORT = TOTAL / 10;   // Report every 10%
    
    auto one_ct = fhe.encrypt(1);
    auto acc = fhe.encrypt(0);
    long double min_noise = acc.noise, max_noise = acc.noise;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::cout << "Running " << (TOTAL/1000000000.0) << "B ops...\n\n";
    
    for(int64_t i = 1; i <= TOTAL; i++) {
        acc = fhe.add(acc, one_ct);
        
        if(acc.noise < min_noise) min_noise = acc.noise;
        if(acc.noise > max_noise) max_noise = acc.noise;
        
        if(i % REPORT == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            int pct = (int)(i * 100 / TOTAL);
            std::cout << "  " << pct << "% | " << (i/1000000000.0) << "B | " 
                      << elapsed << "s | noise: " << (double)min_noise 
                      << "-" << (double)max_noise << "\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    int64_t result = fhe.decrypt(acc);
    
    std::cout << "\n══════════════════════════════════════════════\n";
    std::cout << "  TOTAL: " << TOTAL << " operations\n";
    std::cout << "  TIME: " << sec << "s\n";
    std::cout << "  TPS: " << (TOTAL/(sec+1)/1000000) << "M\n";
    std::cout << "  RESULT: " << result << " (expected " << TOTAL << ")\n";
    std::cout << "  ERROR: " << (TOTAL - result) << "\n";
    std::cout << "  ACCURACY: " << (100.0 - (double)(TOTAL-result)/TOTAL*100.0) << "%\n";
    std::cout << "  NOISE: " << (double)min_noise << " - " << (double)max_noise << "\n";
    std::cout << "══════════════════════════════════════════════\n";
    
    return 0;
}
