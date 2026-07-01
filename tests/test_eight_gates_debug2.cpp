#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <cmath>

using namespace eight_demon_gates;

int main() {
    EightDemonGatesEngine engine;
    
    double x = 42.0;
    std::cout << "Starting: " << x << std::endl;
    
    uint64_t nonce = 0x9E3779B97F4A7C15ULL;
    
    // Manually step through first few engines
    // Engine 0: Fibonacci
    golden_chaos::GoldenChaosEngine fib;
    auto fib_result = fib.observe(x, nonce);
    x = fib_result.first;
    std::cout << "After Fibonacci: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 1: Riemann
    riemann_chaos::RiemannChaosEngine rie;
    auto rie_result = rie.observe(x, nonce);
    x = rie_result.first;
    std::cout << "After Riemann: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 2: Banach
    fibonacci_duel::FibonacciDuelEngine ban;
    auto ban_result = ban.observe(x, nonce);
    x = ban_result.first;
    std::cout << "After Banach: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 3: Gödel
    godel_incompleteness::GodelEngine god;
    x = god.observe(x, nonce);
    std::cout << "After Gödel: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 4: Cantor
    cantor_diagonal::CantorEngine can;
    x = can.observe(x, nonce);
    std::cout << "After Cantor: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 5: Turing
    turing_halting::TuringEngine tur;
    x = tur.observe(x, nonce);
    std::cout << "After Turing: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 6: Heisenberg
    heisenberg_uncertainty::HeisenbergEngine hei;
    x = hei.observe(x, nonce);
    std::cout << "After Heisenberg: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    // Engine 7: Nietzsche
    nietzsche_eternal::NietzscheEngine nie;
    x = nie.observe(x, nonce);
    std::cout << "After Nietzsche: " << x;
    if (std::isnan(x)) std::cout << " ← NAN!";
    std::cout << std::endl;
    
    return 0;
}
