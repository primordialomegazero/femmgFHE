#include "../src/core/femmg_operations.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "======================================================" << std::endl;
    std::cout << "  SMART FHE — Auto-Sensitivity Test" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    struct TestCase {
        std::string name;
        int64_t value;
        FEmmgFHE::Sensitivity sensitivity;
    };
    
    TestCase tests[] = {
        {"Small number (AUTO)",     42,     FEmmgFHE::Sensitivity::AUTO},
        {"Password hash (SENSITIVE)", 0xDEAD, FEmmgFHE::Sensitivity::SENSITIVE},
        {"Master key (CRITICAL)",   0x100,  FEmmgFHE::Sensitivity::CRITICAL},
        {"Large transaction (NORMAL)", 999999, FEmmgFHE::Sensitivity::NORMAL},
        {"Zero (AUTO)",             0,      FEmmgFHE::Sensitivity::AUTO},
        {"Negative (AUTO)",         -42,    FEmmgFHE::Sensitivity::AUTO},
    };
    
    for (auto& t : tests) {
        std::cout << "\n--- " << t.name << " ---" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto ct = fhe.encrypt_smart(t.value, t.sensitivity);
        int64_t dec = fhe.decrypt_smart(ct);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "  Value: " << t.value << " -> Decrypt: " << dec 
                  << (dec == t.value ? " OK" : " FAIL") << std::endl;
        std::cout << "  Time: " << us << " µs" << std::endl;
        std::cout << "  IV: 0x" << std::hex << ct.random_iv << std::dec << std::endl;
    }
    
    // Performance comparison
    std::cout << "\n======================================================" << std::endl;
    std::cout << "  PERFORMANCE COMPARISON (1000 ops each)" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) fhe.encrypt_smart(i, FEmmgFHE::Sensitivity::NORMAL);
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "  NORMAL (depth=1):  " << (us / 1000.0) << " µs/op" << std::endl;
    }
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) fhe.encrypt_smart(i, FEmmgFHE::Sensitivity::SENSITIVE);
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "  SENSITIVE (depth=3): " << (us / 1000.0) << " µs/op" << std::endl;
    }
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) fhe.encrypt_smart(i, FEmmgFHE::Sensitivity::CRITICAL);
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "  CRITICAL (depth=7):  " << (us / 1000.0) << " µs/op" << std::endl;
    }
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) fhe.encrypt_smart(i, FEmmgFHE::Sensitivity::AUTO);
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "  AUTO (smart):      " << (us / 1000.0) << " µs/op" << std::endl;
    }
    
    return 0;
}
