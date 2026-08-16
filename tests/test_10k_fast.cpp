#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "10K NAND (FAST - 128-bit Q for testing)\n\n";
    
    // Mas maliit na Q para sa mabilis na testing
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297"); // 128-bit prime
    
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    int errors = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    auto current = fhe.encrypt(true);
    for (int i = 1; i <= 10000; i++) {
        current = fhe.nand_gate(current, current);
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0);
        if (result != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "Error at " << i << ": got " << result 
                          << ", exp " << expected << "\n";
            }
        }
        if (i % 1000 == 0) {
            std::cout << "  Progress: " << i << "/10000, errors=" << errors << "\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== RESULTS ===\n";
    std::cout << "Operations: 10000\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Time: " << elapsed << "s\n";
    std::cout << "Ops/sec: " << (10000 / elapsed) << "\n";
    
    return 0;
}
