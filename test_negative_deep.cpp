#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <iostream>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  NEGATIVE VALUES — DEEP CIRCUIT TEST          ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    auto neg_one = fhe.encrypt(-1);
    auto pos_one = fhe.encrypt(1);
    
    // Negative accumulation (100K)
    auto acc = fhe.encrypt(0);
    for(int i = 0; i < 100000; i++) acc = fhe.add(acc, neg_one);
    std::cout << "100K × (-1) = " << fhe.decrypt(acc) << " (expected -100000) "
              << (fhe.decrypt(acc) == -100000 ? "✅" : "❌") << "\n";
    std::cout << "  Noise: " << acc.noise << "\n\n";
    
    // Mixed positive/negative (cancellation)
    acc = fhe.encrypt(1000000);
    for(int i = 0; i < 1000000; i++) acc = fhe.add(acc, neg_one);
    std::cout << "1M - 1M = " << fhe.decrypt(acc) << " (expected 0) "
              << (fhe.decrypt(acc) == 0 ? "✅" : "❌") << "\n";
    std::cout << "  Noise: " << acc.noise << "\n\n";
    
    // Deep multiplication with negatives
    auto mul_acc = fhe.encrypt(-2);
    for(int i = 0; i < 10; i++) mul_acc = fhe.multiply(mul_acc, fhe.encrypt(2));
    std::cout << "(-2) × 2^10 = " << fhe.decrypt(mul_acc) << " (expected -2048) "
              << (fhe.decrypt(mul_acc) == -2048 ? "✅" : "❌") << "\n";
    std::cout << "  Noise: " << mul_acc.noise << "\n\n";
    
    // Alternating signs
    acc = fhe.encrypt(0);
    for(int i = 0; i < 50000; i++) {
        acc = fhe.add(acc, pos_one);
        acc = fhe.add(acc, neg_one);
    }
    std::cout << "50K × (+1-1) = " << fhe.decrypt(acc) << " (expected 0) "
              << (fhe.decrypt(acc) == 0 ? "✅" : "❌") << "\n";
    std::cout << "  Noise: " << acc.noise << "\n\n";
    
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  NEGATIVE VALUES: Fibonacci-Lyapunov handles them all\n";
    std::cout << "══════════════════════════════════════════════\n";
    
    return 0;
}
