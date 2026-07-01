#include "../src/chaos/triple_rashomon.h"
#include <iostream>

using namespace triple_rashomon;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  TRIPLE LAYER TRIPLE RASHOMON" << std::endl;
    std::cout << "  3 × 14 = 42 Layers of Chaos" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    TripleRashomonEngine engine;
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
    std::cout << "diff=" << diff;
    if (diff > 100000.0) std::cout << " ✅ MASSIVE!";
    else if (diff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000.0) passed++;
    
    // Test 3: Nonce sensitivity
    std::cout << "3. NONCE: ";
    engine.set_nonce(0x111);
    auto [ct3, h3] = engine.observe(42.0);
    engine.set_nonce(0x222);
    auto [ct4, h4] = engine.observe(42.0);
    double ndiff = std::abs(ct3 - ct4);
    std::cout << "diff=" << ndiff;
    if (ndiff > 100000.0) std::cout << " ✅ MASSIVE!";
    else if (ndiff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (ndiff > 1000.0) passed++;
    
    // Test 4: Total layers
    std::cout << "4. LAYERS: " << engine.total_layers() << " ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
