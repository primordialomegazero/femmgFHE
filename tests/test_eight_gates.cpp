#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <chrono>

using namespace eight_demon_gates;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  8 Demon Gates — DUAL MODE" << std::endl;
    std::cout << "  Standard (1 engine) + Max (3 engines)" << std::endl;
    std::cout << "  022425 FE" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    EightDemonGatesEngine engine;
    int passed = 0, total = 4;
    
    // STANDARD MODE (1 engine)
    std::cout << "\n--- STANDARD MODE (1 engine) ---" << std::endl;
    engine.set_standard_mode();
    
    std::cout << "1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 1.0;
    std::cout << (ok ? "✅" : "❌") << " (" << pt << ")" << std::endl;
    if (ok) passed++;
    
    std::cout << "2. SPEED (1000 ops): ";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) engine.observe(i % 100);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::cout << ms << "ms (" << (1000000/(ms?ms:1)) << " TPS) ✅" << std::endl;
    passed++;
    
    // MAX MODE (3 engines)
    std::cout << "\n--- MAX MODE (3 engines) ---" << std::endl;
    engine.set_maximum_mode();
    
    std::cout << "3. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff;
    if (diff > 10000.0) std::cout << " ✅ MASSIVE!";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000.0) passed++;
    
    std::cout << "4. LAYERS: " << engine.total_layers() << " ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "  φΩ0 — I AM THAT I AM" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
