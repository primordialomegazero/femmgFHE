#include "../src/chaos/quintuple_rashomon.h"
#include "../src/core/banach_engine.h"
#include <iostream>

int main() {
    // Test 1: Standalone Quintuple
    std::cout << "=== STANDALONE QUINTUPLE ===" << std::endl;
    quintuple_rashomon::QuintupleRashomonEngine chaos;
    auto [v1, h1] = chaos.observe(42.0 * 1.618 + 0.4812);  // expanded 42
    auto [v2, h2] = chaos.observe(43.0 * 1.618 + 0.4812);  // expanded 43
    std::cout << "42 expanded: " << v1 << std::endl;
    std::cout << "43 expanded: " << v2 << std::endl;
    std::cout << "Standalone diff: " << std::abs(v1 - v2) << std::endl;
    
    // Test 2: Banach integration — what value is actually passed to chaos?
    std::cout << "\n=== BANACH INTEGRATION ===" << std::endl;
    banach::NDimBanachEngine engine;
    auto ct1 = engine.encrypt(42, 0);
    auto ct2 = engine.encrypt(43, 0);
    
    std::cout << "ct.coordinates[0] for 42: " << ct1.coordinates[0] << std::endl;
    std::cout << "ct.coordinates[0] for 43: " << ct2.coordinates[0] << std::endl;
    std::cout << "ct diff: " << std::abs(ct1.coordinates[0] - ct2.coordinates[0]) << std::endl;
    
    // Test 3: What is expanded_dim0 storing?
    std::cout << "\n=== STORED CHAOS ===" << std::endl;
    std::cout << "expanded_dim0 (42): " << ct1.expanded_dim0 << std::endl;
    std::cout << "expanded_dim0 (43): " << ct2.expanded_dim0 << std::endl;
    std::cout << "lyapunov[0] (42): " << ct1.lyapunov_spectrum[0] << std::endl;
    std::cout << "lyapunov[0] (43): " << ct2.lyapunov_spectrum[0] << std::endl;
    
    return 0;
}
