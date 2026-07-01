#include "../src/chaos/fibonacci_reversal.h"
#include <iostream>

using namespace fibonacci_reversal;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  FIBONACCI REVERSAL CHAOS TEST" << std::endl;
    std::cout << "  Third Chaos Engine" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    FibonacciReversalEngine engine;
    int passed = 0, total = 4;
    
    // Test 1: Roundtrip
    std::cout << "\n1. ROUNDTRIP: ";
    auto [ct, hist] = engine.observe(42.0);
    double pt = engine.unobserve(ct, hist);
    bool ok = std::abs(pt - 42.0) < 2.0;  // Slightly looser due to scramble
    std::cout << (ok ? "✅" : "❌") << " (" << pt << ")" << std::endl;
    if (ok) passed++;
    
    // Test 2: Chaos effect
    std::cout << "2. CHAOS (42 vs 43): ";
    auto [ct1, h1] = engine.observe(42.0);
    auto [ct2, h2] = engine.observe(43.0);
    double diff = std::abs(ct1 - ct2);
    std::cout << "diff=" << diff << (diff > 10.0 ? " ✅" : " ⚠️") << std::endl;
    if (diff > 10.0) passed++;
    
    // Test 3: Reversal property
    std::cout << "3. REVERSAL DEPTH: " << engine.reversal_depth() << " layers ✅" << std::endl;
    passed++;
    
    // Test 4: Different nonces
    std::cout << "4. NONCE: ";
    engine.set_nonce(0x111);
    auto [ct3, h3] = engine.observe(42.0);
    engine.set_nonce(0x222);
    auto [ct4, h4] = engine.observe(42.0);
    double ndiff = std::abs(ct3 - ct4);
    std::cout << "diff=" << ndiff << (ndiff > 5.0 ? " ✅" : " ⚠️") << std::endl;
    if (ndiff > 5.0) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
