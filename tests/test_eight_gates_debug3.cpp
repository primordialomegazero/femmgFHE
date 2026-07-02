#include "../src/chaos/eight_demon_gates.h"
#include <iostream>

using namespace eight_demon_gates;

int main() {
    EightDemonGatesEngine engine;
    engine.set_maximum_mode();
    
    // Test each engine individually with 42 and 43
    std::cout << "Testing each engine with 42 vs 43:" << std::endl;
    
    // Fibonacci
    {
        golden_chaos::GoldenChaosEngine eng;
        auto v42 = eng.observe(42.0 * 10000.0, 0).first;
        auto v43 = eng.observe(43.0 * 10000.0, 0).first;
        std::cout << "  Fibonacci: 42=" << v42 << " 43=" << v43 << " diff=" << std::abs(v42-v43) << std::endl;
    }
    
    // Riemann
    {
        riemann_chaos::RiemannChaosEngine eng;
        auto v42 = eng.observe(42.0 * 10000.0, 0).first;
        auto v43 = eng.observe(43.0 * 10000.0, 0).first;
        std::cout << "  Riemann:   42=" << v42 << " 43=" << v43 << " diff=" << std::abs(v42-v43) << std::endl;
    }
    
    // Gödel
    {
        godel_incompleteness::GodelEngine eng;
        double v42 = eng.observe(42.0 * 10000.0, 0);
        double v43 = eng.observe(43.0 * 10000.0, 0);
        std::cout << "  Gödel:     42=" << v42 << " 43=" << v43 << " diff=" << std::abs(v42-v43) << std::endl;
    }
    
    // Cantor
    {
        cantor_diagonal::CantorEngine eng;
        double v42 = eng.observe(42.0 * 10000.0, 0);
        double v43 = eng.observe(43.0 * 10000.0, 0);
        std::cout << "  Cantor:    42=" << v42 << " 43=" << v43 << " diff=" << std::abs(v42-v43) << std::endl;
    }
    
    return 0;
}
