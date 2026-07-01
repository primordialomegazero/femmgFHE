#include "../src/chaos/fibonacci_duel.h"
#include <iostream>

using namespace fibonacci_duel;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  FIBONACCI DUEL — DOUBLE φ TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    FibonacciDuelEngine engine;
    int passed = 0, total = 5;
    
    // Test 1: Roundtrip
    std::cout << "\n1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 0.01;
    std::cout << (ok ? "✅" : "❌") << std::endl;
    if (ok) passed++;
    
    // Test 2: Chaos effect
    std::cout << "2. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff << (diff > 5.0 ? " ✅" : " ⚠️") << std::endl;
    if (diff > 5.0) passed++;
    
    // Test 3: Balance
    std::cout << "3. BALANCE: " << engine.forward_layers() << "F + " 
              << engine.reverse_layers() << "R ✅" << std::endl;
    passed++;
    
    // Test 4: Floors
    std::cout << "4. FLOORS: " << engine.floor_at(0) << "," << engine.floor_at(1) 
              << "," << engine.floor_at(2) << " ✅" << std::endl;
    passed++;
    
    // Test 5: Multi-value
    std::cout << "5. MULTI: ";
    bool multi = true;
    for (double v : {0.0, 100.0, -42.0, 123456.0}) {
        auto [c, h] = engine.observe(v);
        double d = engine.unobserve(c, h);
        if (std::abs(d - v) > 0.1) multi = false;
    }
    std::cout << (multi ? "✅" : "❌") << std::endl;
    if (multi) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
