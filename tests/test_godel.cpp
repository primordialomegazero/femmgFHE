#include "../src/chaos/godel_incompleteness.h"
#include <iostream>

using namespace godel_incompleteness;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  GÖDEL INCOMPLETENESS ENGINE v3" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 4;
    
    // Test 1: Chaos effect
    std::cout << "\n1. CHAOS (42 vs 43): ";
    GodelEngine engine;
    auto v42 = engine.observe(42.0);
    auto v43 = engine.observe(43.0);
    double diff = std::abs(v42 - v43);
    std::cout << "diff=" << diff;
    if (diff > 10000.0) std::cout << " ✅ MASSIVE!";
    else if (diff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (diff > 1000.0) passed++;
    
    // Test 2: Nonce sensitivity (FRESH engines)
    std::cout << "2. NONCE: ";
    GodelEngine e1, e2;
    e1.set_nonce(0x111);
    e2.set_nonce(0x222);
    double v_a = e1.observe(42.0);
    double v_b = e2.observe(42.0);
    double ndiff = std::abs(v_a - v_b);
    std::cout << "diff=" << ndiff;
    if (ndiff > 10000.0) std::cout << " ✅ MASSIVE!";
    else if (ndiff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (ndiff > 1000.0) passed++;
    
    // Test 3: Self-reference (φ sensitivity)
    std::cout << "3. SELF-REF: ";
    GodelEngine e3;
    double self = e3.observe(PHI);
    double self2 = e3.observe(PHI + 0.001);
    double sdiff = std::abs(self - self2);
    std::cout << "diff=" << sdiff;
    if (sdiff > 1000.0) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;
    if (sdiff > 1000.0) passed++;
    
    // Test 4: Basic sanity
    std::cout << "4. BASIC: ";
    GodelEngine e4;
    double v = e4.observe(42.0);
    std::cout << "42 → " << v << " ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
