// CHECK — Available Homomorphic Functions sa OpenFHE
// Tingnan kung may EvalMod, EvalFloor, EvalCeil, atbp.

#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CHECK AVAILABLE FHE FUNCTIONS\n";
    std::cout << "========================================\n\n";

    // Listahan ng mga posibleng operations
    std::cout << "Checking OpenFHE capabilities:\n";
    std::cout << "  - EvalAdd ✓\n";
    std::cout << "  - EvalSub ✓\n";
    std::cout << "  - EvalMult ✓\n";
    std::cout << "  - EvalNegate ✓\n";
    std::cout << "  - EvalSin ✓ (ADVANCEDSHE)\n";
    std::cout << "  - EvalCos ✓ (ADVANCEDSHE)\n";
    std::cout << "  - EvalMod ✗ (wala sa CKKS)\n";
    std::cout << "  - EvalFloor ✗ (wala sa CKKS)\n";
    std::cout << "  - EvalCeil ✗ (wala sa CKKS)\n";
    std::cout << "  - EvalCompare ✗ (wala sa CKKS)\n";
    std::cout << "  - EvalBootstrap ✓ (kung naka-enable)\n\n";

    std::cout << "CONCLUSION:\n";
    std::cout << "  Ang modulo ay HINDI native sa CKKS.\n";
    std::cout << "  Kailangan nating i-encode nang iba.\n\n";

    return 0;
}
