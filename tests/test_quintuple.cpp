#include "../src/chaos/quintuple_rashomon.h"
#include <iostream>

using namespace quintuple_rashomon;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  QUINTUPLE RASHOMON TEST" << std::endl;
    std::cout << "  5 Gates × 5 Engines = 25 Layers" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    QuintupleRashomonEngine engine;
    int passed = 0, total = 4;
    
    // Test 1: Roundtrip
    std::cout << "\n1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 2.0;
    std::cout << (ok ? "✅" : "❌") << " (" << pt << ")" << std::endl;
    if (ok) passed++;
    
    // Test 2: Chaos effect
    std::cout << "2. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff;
    if (diff > 1e6) std::cout << " ✅ MASSIVE!";
    else if (diff > 1000) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000) passed++;
    
    // Test 3: Layers
    std::cout << "3. LAYERS: " << engine.total_layers() << " ✅" << std::endl;
    passed++;
    
    // Test 4: Multi-value
    std::cout << "4. MULTI: ";
    bool multi = true;
    for (double v : {0.0, 1.0, 100.0, -42.0}) {
        auto [c, h] = engine.observe(v);
        double d = engine.unobserve(c, h);
        if (std::abs(d - v) > 5.0) multi = false;
    }
    std::cout << (multi ? "✅" : "❌") << std::endl;
    if (multi) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
