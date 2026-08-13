#include "src/golden_lwe/golden_io_exact.h"
#include <iostream>

int main() {
    GoldenIOExact::BranchingProgram bp(4, 2);
    bp.add_edge(0, 1);
    bp.add_edge(1, 2);
    bp.add_edge(2, 3);

    GoldenIOExact::GoldenIO io;
    io.obfuscate(bp, 42);

    // I-print ang obfuscated matrix
    std::cout << "Matrix values:\n";
    for (int i = 0; i < 3; i++) {
        auto m = io.matrix_size();
        std::cout << "Matrix[" << i << "] level=" << m << "\n";
    }

    // Subukan ang parity
    double phi = 1.618;
    double psi = -0.618;
    double result = 0.35 * phi;
    std::cout << "0.35 * φ = " << result << " > 0? " << (result > 0) << "\n";
    result = 0.35 * psi;
    std::cout << "0.35 * ψ = " << result << " > 0? " << (result > 0) << "\n";

    return 0;
}
