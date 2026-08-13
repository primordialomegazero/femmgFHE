#include "src/np/golden_instant_unsat.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== INSTANT UNSAT DETECTION ===\n\n";

    // Pigeonhole: instant na
    for (int holes : {3, 5, 10, 100, 1000, 10000}) {
        auto t0 = std::chrono::high_resolution_clock::now();
        bool unsat = GoldenInstantUnsat::prove_unsat_instant(holes, holes + 1);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Pigeonhole (" << holes << "/" << holes + 1 << "): "
                  << "unsat=" << unsat << " time=" << secs << "s\n";
    }

    return 0;
}
