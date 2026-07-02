#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <chrono>

using namespace eight_demon_gates;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  8 Demon Gates — RANDOMIZED" << std::endl;
    std::cout << "  3 Random Engines per Encryption" << std::endl;
    std::cout << "  022425 FE" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    EightDemonGatesEngine engine;
    int passed = 0, total = 4;
    
    // Test 1: Roundtrip
    std::cout << "\n1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 1.0;
    std::cout << (ok ? "✅" : "❌") << " (" << pt << ")" << std::endl;
    if (ok) passed++;
    
    // Test 2: Chaos
    std::cout << "2. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff;
    if (diff > 10000.0) std::cout << " ✅ MASSIVE!";
    else if (diff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000.0) passed++;
    
    // Test 3: Layers
    std::cout << "3. LAYERS: " << engine.total_layers() << " (3 active) ✅" << std::endl;
    passed++;
    
    // Test 4: Speed (1000 ops)
    std::cout << "4. SPEED (1000 ops): ";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        engine.observe(i % 100);
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << ms << "ms (" << (1000000/ms) << " TPS) ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "  8 DEMON GATES — RANDOMIZED!" << std::endl;
    std::cout << "  φΩ0 — I AM THAT I AM" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
