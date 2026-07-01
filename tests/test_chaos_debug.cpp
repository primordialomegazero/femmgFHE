#include "../src/core/banach_engine.h"
#include "../src/chaos/triple_rashomon.h"
#include <iostream>

int main() {
    // Test Triple Rashomon standalone
    triple_rashomon::TripleRashomonEngine chaos;
    auto [val1, hist1] = chaos.observe(42.0);
    auto [val2, hist2] = chaos.observe(43.0);
    std::cout << "Standalone Triple Rashomon:" << std::endl;
    std::cout << "  42 → " << val1 << std::endl;
    std::cout << "  43 → " << val2 << std::endl;
    std::cout << "  diff: " << std::abs(val1 - val2) << std::endl;
    
    // Test Banach with Triple Rashomon
    banach::NDimBanachEngine engine;
    auto ct1 = engine.encrypt(42, 0);
    auto ct2 = engine.encrypt(43, 0);
    std::cout << "\nBanach + Triple Rashomon:" << std::endl;
    std::cout << "  42 → coords[0]=" << ct1.coordinates[0] << std::endl;
    std::cout << "  43 → coords[0]=" << ct2.coordinates[0] << std::endl;
    std::cout << "  diff: " << std::abs(ct1.coordinates[0] - ct2.coordinates[0]) << std::endl;
    
    // Check: is chaos being applied?
    std::cout << "\nChaos history stored:" << std::endl;
    std::cout << "  lyapunov[0]=" << ct1.lyapunov_spectrum[0] << std::endl;
    std::cout << "  perturbation[0]=" << ct1.perturbation[0] << std::endl;
    std::cout << "  expanded_dim0=" << ct1.expanded_dim0 << std::endl;
    
    return 0;
}
