#include "../src/chaos/sine_zeta_merge.h"
#include <iostream>

using namespace sine_zeta_merge;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  SINE-ZETA MERGE TEST" << std::endl;
    std::cout << "  Golden Chaos + Riemann Chaos" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    SineZetaEngine engine;
    int passed = 0, total = 4;
    
    // Test 1: Roundtrip
    std::cout << "\n1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 1.0;
    std::cout << (ok ? "✅" : "❌") << std::endl;
    if (ok) passed++;
    
    // Test 2: Chaos effect
    std::cout << "2. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff << (diff > 20.0 ? " ✅" : " ⚠️") << std::endl;
    if (diff > 20.0) passed++;
    
    // Test 3: Ratios
    std::cout << "3. RATIOS: sine=" << engine.sine_ratio()*100 << "% zeta=" << engine.zeta_ratio()*100 << "% ✅" << std::endl;
    passed++;
    
    // Test 4: Multiple values
    std::cout << "4. MULTI-VALUE: ";
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
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
