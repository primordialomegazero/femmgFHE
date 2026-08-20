// TFHE + φ-STRUCTURE — Tamang API
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TFHE / BINFHE CHECK\n";
    std::cout << "========================================\n\n";

    // I-check ang BinFHE (Binary FHE - ito ang TFHE variant sa OpenFHE)
    try {
        auto cc = BinFHEContext();
        std::cout << "BinFHE Context: AVAILABLE ✓\n";
        std::cout << "  Ito ang binary FHE sa OpenFHE\n";
        std::cout << "  May native NAND, AND, OR, XOR gates\n";
        std::cout << "  Bootstrapping per gate (unlimited depth)\n\n";
    } catch (...) {
        std::cout << "BinFHE: NOT AVAILABLE\n";
    }

    return 0;
}
