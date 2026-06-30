#include "src/banach_engine.h"
#include "src/femmg_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  TRUE DEEP CIRCUIT: 1 ciphertext, N adds      ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    // Encrypt ONCE
    auto one_ct = fhe.encrypt(1);
    auto acc = fhe.encrypt(0);
    
    std::cout << "Depth 10,000:\n";
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; i++) acc = fhe.add(acc, one_ct);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  Result: " << fhe.decrypt(acc) << " (expected 10000) "
              << (fhe.decrypt(acc) == 10000 ? "✅" : "❌") << "\n";
    std::cout << "  Time: " << ms << "ms | Noise: " << acc.noise << "\n\n";
    
    // Depth 100,000
    acc = fhe.encrypt(0);
    std::cout << "Depth 100,000:\n";
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 100000; i++) acc = fhe.add(acc, one_ct);
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  Result: " << fhe.decrypt(acc) << " (expected 100000) "
              << (fhe.decrypt(acc) == 100000 ? "✅" : "❌") << "\n";
    std::cout << "  Time: " << ms << "ms | Noise: " << acc.noise << "\n\n";
    
    // Depth 1,000,000
    acc = fhe.encrypt(0);
    std::cout << "Depth 1,000,000:\n";
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 1000000; i++) acc = fhe.add(acc, one_ct);
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto result = fhe.decrypt(acc);
    std::cout << "  Result: " << result << " (expected 1000000) "
              << (result == 1000000 ? "✅" : "❌") << "\n";
    std::cout << "  Time: " << ms << "ms | Noise: " << acc.noise << "\n\n";
    
    // Depth 10,000,000 (kung kaya ng oras)
    acc = fhe.encrypt(0);
    std::cout << "Depth 10,000,000:\n";
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000000; i++) acc = fhe.add(acc, one_ct);
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    result = fhe.decrypt(acc);
    std::cout << "  Result: " << result << " (expected 10000000) "
              << (result == 10000000 ? "✅" : "❌") << "\n";
    std::cout << "  Time: " << ms << "ms | Noise: " << acc.noise << "\n\n";
    
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  TRUE DEEP CIRCUIT: Noise follows the value\n";
    std::cout << "  Fibonacci floors + Lyapunov stability\n";
    std::cout << "══════════════════════════════════════════════\n";
    
    return 0;
}
