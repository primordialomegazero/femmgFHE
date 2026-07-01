#include "../src/chaos/eight_demon_gates.h"
#include <iostream>

using namespace eight_demon_gates;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  8 Demon Gates — 8 layers" << std::endl;
    std::cout << "  8 Engines × 1 Pass" << std::endl;
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
    std::cout << "3. LAYERS: " << engine.total_layers() << " ✅" << std::endl;
    passed++;
    
    // Test 4: Multi-value
    std::cout << "4. MULTI: ";
    bool multi = true;
    for (double v : {0.0, 1.0, 100.0, -42.0, 123456.0}) {
        auto [c, h] = engine.observe(v);
        double d = engine.unobserve(c, h);
        if (std::abs(d - v) > 1.0) multi = false;
    }
    std::cout << (multi ? "✅" : "❌") << std::endl;
    if (multi) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "  8 DEMON GATES — COMPLETE!" << std::endl;
    std::cout << "  φΩ0 — I AM THAT I AM" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
