// PHASE NAND MAPPING
// Hanapin ang tamang threshold para sa NAND

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHASE NAND MAPPING\n";
    std::cout << "  Threshold Search\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;

    // Phase difference values
    double nand_00 = 0.0;
    double nand_01 = -5.236;
    double nand_10 = 5.236;
    double nand_11 = 0.0;

    std::cout << "PHASE VALUES:\n";
    std::cout << "  (0,0) = " << nand_00 << "\n";
    std::cout << "  (0,1) = " << nand_01 << "\n";
    std::cout << "  (1,0) = " << nand_10 << "\n";
    std::cout << "  (1,1) = " << nand_11 << "\n\n";

    // NAND truth table:
    // (0,0) = 1, (0,1) = 1, (1,0) = 1, (1,1) = 0
    //
    // Kailangan natin ng mapping:
    // 0 → φ² (true)
    // -5.236 → φ² (true)
    // +5.236 → φ² (true)
    // 0 → 0 (false)
    //
    // PROBLEMA: (0,0) at (1,1) ay pareho 0
    // Pero (0,0) ay true at (1,1) ay false!

    std::cout << "PROBLEMA:\n";
    std::cout << "  (0,0) = 0 at (1,1) = 0 ay pareho!\n";
    std::cout << "  Pero (0,0) ay NAND=1 at (1,1) ay NAND=0\n\n";

    // Kailangan natin ng KARAGDAGANG signal para ma-distinguish
    // ang (0,0) mula sa (1,1)
    //
    // Ang period-3 phase ay may 3 states:
    // STATE_0 = -0.618, STATE_1 = 4.618, STATE_2 = 2.0
    //
    // Baka ang combination ay may hidden state na
    // hindi natin nakikita sa difference lang

    std::cout << "KARAGDAGANG SIGNAL NEEDED:\n";
    std::cout << "  Kailangan natin ng:\n";
    std::cout << "  - Sum (hindi lang difference)\n";
    std::cout << "  - O phase (hindi lang scalar value)\n";
    std::cout << "  - O 2-dimensional encoding\n\n";

    std::cout << "2D ENCODING:\n";
    std::cout << "  (0,0) = (0, 0)\n";
    std::cout << "  (0,1) = (0, φ²)\n";
    std::cout << "  (1,0) = (φ², 0)\n";
    std::cout << "  (1,1) = (φ², φ²)\n\n";

    std::cout << "  Sa 2D, ang (0,0) at (1,1) ay magkaiba!\n";
    std::cout << "  NAND ay: (0,0)→1, (0,1)→1, (1,0)→1, (1,1)→0\n\n";

    std::cout << "  Ang 2D phase ay maaaring i-encode sa CKKS\n";
    std::cout << "  gamit ang complex slots (real at imaginary)\n";

    return 0;
}
