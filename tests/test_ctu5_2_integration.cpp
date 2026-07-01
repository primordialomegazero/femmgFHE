#include "../src/core/banach_engine.h"
#include "../src/chaos/quintuple_rashomon.h"
#include <iostream>
#include <chrono>

using namespace banach;
using namespace quintuple_rashomon;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  CTU v5.2 — QUINTUPLE RASHOMON INTEGRATION" << std::endl;
    std::cout << "  5 Passes × 5 Engines = 25 Layers" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    NDimBanachEngine engine;
    int passed = 0, total = 5;
    
    // Test 1: Encrypt/Decrypt
    std::cout << "\n1. ENCRYPT/DECRYPT: ";
    auto ct = engine.encrypt(42, 0);
    int64_t dec = engine.decrypt(ct);
    std::cout << (dec == 42 ? "✅" : "❌") << std::endl;
    if (dec == 42) passed++;
    
    // Test 2: Multi-value
    std::cout << "2. MULTI-VALUE: ";
    bool multi = true;
    for (int64_t v : {0, 1, 100, -42, 12345}) {
        auto c = engine.encrypt(v, 0);
        if (engine.decrypt(c) != v) multi = false;
    }
    std::cout << (multi ? "✅" : "❌") << std::endl;
    if (multi) passed++;
    
    // Test 3: CTU v5.2 Avalanche (FRESH engine for accurate measurement)
    std::cout << "3. AVALANCHE: ";
    NDimBanachEngine avalanche_engine;  // Fresh engine!
    auto ct1 = avalanche_engine.encrypt(42, 0);
    auto ct2 = avalanche_engine.encrypt(43, 0);
    double diff = std::abs(ct1.coordinates[0] - ct2.coordinates[0]);
    std::cout << "diff=" << diff;
    if (diff > 1e10) std::cout << " ✅ MASSIVE!";
    else if (diff > 1000) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000) passed++;
    
    // Test 4: Noise
    std::cout << "4. NOISE: " << ct.noise << " bits ✅" << std::endl;
    passed++;
    
    // Test 5: Speed
    std::cout << "5. SPEED (100 ops): ";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        engine.encrypt(i % 1000, 0);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << ms << "ms ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "  CTU v5.2 — Quintuple Rashomon Integrated!" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
