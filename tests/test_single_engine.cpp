#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Testing each engine (1000 ops each)..." << std::endl;
    
    // Engine 0: Fibonacci
    {
        golden_chaos::GoldenChaosEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Fibonacci: " << ms << "ms" << std::endl;
    }
    
    // Engine 1: Riemann
    {
        riemann_chaos::RiemannChaosEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Riemann:  " << ms << "ms" << std::endl;
    }
    
    // Engine 2: Banach
    {
        fibonacci_duel::FibonacciDuelEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Banach:   " << ms << "ms" << std::endl;
    }
    
    // Engine 3: Gödel
    {
        godel_incompleteness::GodelEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Gödel:    " << ms << "ms" << std::endl;
    }
    
    // Engine 4: Cantor
    {
        cantor_diagonal::CantorEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Cantor:   " << ms << "ms" << std::endl;
    }
    
    // Engine 5: Turing
    {
        turing_halting::TuringEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Turing:   " << ms << "ms" << std::endl;
    }
    
    // Engine 6: Heisenberg
    {
        heisenberg_uncertainty::HeisenbergEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Heisenberg: " << ms << "ms" << std::endl;
    }
    
    // Engine 7: Nietzsche
    {
        nietzsche_eternal::NietzscheEngine eng;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) eng.observe(i % 100, i);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "  Nietzsche: " << ms << "ms" << std::endl;
    }
    
    std::cout << "\nDone! Check which engine is the bottleneck." << std::endl;
    return 0;
}
