#include "src/golden_lwe/golden_io_exact.h"
#include <iostream>

int main() {
    std::cout << "=== GOLDEN iO EXACT (FGG MULTILINEAR) ===\n\n";

    GoldenIOExact::BranchingProgram bp(4, 2);
    bp.add_edge(0, 1);
    bp.add_edge(1, 2);
    bp.add_edge(2, 3);

    GoldenIOExact::GoldenIO io;
    io.obfuscate(bp, 42);

    std::cout << "Obfuscated matrix size: " << io.matrix_size() << "\n";
    std::cout << "Evaluate (true,false): " << io.evaluate({true, false}) << " (expect 1)\n";
    std::cout << "Evaluate (false,false): " << io.evaluate({false, false}) << " (expect 0)\n";

    // Multilinear test
    GoldenIOExact::GoldenMultilinearEncoding m1(0.5, 1);
    GoldenIOExact::GoldenMultilinearEncoding m2(0.7, 2);
    auto m3 = m1 * m2;
    std::cout << "\nMultilinear: level " << m3.get_level() << ", value " << m3.get_value() << "\n";
    std::cout << "Zero-test: " << m3.is_zero() << "\n";

    return 0;
}
