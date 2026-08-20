#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FERMAT PRIME STRUCTURE ANALYSIS\n";
    std::cout << "  65537 = 2^16 + 1\n";
    std::cout << "  Multiplicative group order: 2^16\n";
    std::cout << "========================================\n\n";

    // Fermat prime properties
    uint64_t p = 65537;
    uint64_t group_order = p - 1; // 65536 = 2^16
    
    std::cout << "1. GROUP ORDER DECOMPOSITION:\n";
    std::cout << "   p - 1 = " << group_order << " = 2^16\n";
    std::cout << "   Lahat ng elements ay may order na power of 2\n\n";

    // Hanapin ang primitive root
    uint64_t primitive_root = 3; // 3 ay primitive root ng 65537
    
    std::cout << "2. POWERS OF PRIMITIVE ROOT (3):\n";
    std::cout << "   (Dapat cyclic na may period 65536)\n";
    std::cout << "-----------------------------------\n";
    
    uint64_t current = 1;
    for (int i = 0; i <= 16; i++) {
        std::cout << "   3^" << i << " = " << current << "\n";
        current = (current * primitive_root) % p;
    }

    std::cout << "\n3. NOISE THRESHOLD vs GROUP ORDER:\n";
    std::cout << "   Noise threshold = p/2 = " << (p/2) << "\n";
    std::cout << "   Group order = 2^16 = " << group_order << "\n";
    std::cout << "   Relationship: p/2 = 2^15 + 1\n\n";

    // Ang NAND ay gumagamit ng multiplication
    // Sa group theory, ang multiplication ay repeated addition
    // sa exponent domain
    std::cout << "4. DISCRETE LOG ANALYSIS:\n";
    std::cout << "   (NAND sa exponent domain)\n";
    std::cout << "-----------------------------------\n";

    // Compute discrete logs base 3
    std::vector<int64_t> discrete_log(p, -1);
    current = 1;
    for (int i = 0; i < p - 1; i++) {
        discrete_log[current] = i;
        current = (current * primitive_root) % p;
    }

    // NAND(0,0) = 1, NAND(0,1) = 1, NAND(1,0) = 1, NAND(1,1) = 0
    // Sa exponent domain:
    // 0 ay walang discrete log (undefined)
    // 1 ay may log 0
    // NAND(x,y) = 1 - xy

    std::cout << "   Discrete log ng 1: " << discrete_log[1] << "\n";
    std::cout << "   Discrete log ng -1: " << discrete_log[p-1] << "\n";
    std::cout << "   Discrete log ng 0: UNDEFINED\n\n";

    // Ang problema: 0 ay walang discrete log
    // Kaya ang NAND ay hindi directly expressible
    // sa exponent domain

    std::cout << "5. QUADRATIC RESIDUE ANALYSIS:\n";
    std::cout << "-----------------------------------\n";
    
    // Ang 1 ay laging quadratic residue
    // Ang -1 ay quadratic residue kung p ≡ 1 mod 4
    // 65537 ≡ 1 mod 4, kaya -1 ay quadratic residue
    
    std::cout << "   p ≡ " << (p % 4) << " mod 4\n";
    std::cout << "   -1 ay quadratic residue: ";
    
    // Hanapin ang sqrt(-1)
    uint64_t sqrt_minus_1 = 0;
    for (uint64_t i = 1; i < p; i++) {
        if ((i * i) % p == p - 1) {
            sqrt_minus_1 = i;
            break;
        }
    }
    std::cout << "sqrt(-1) = " << sqrt_minus_1 << "\n";
    std::cout << "   Verification: " << sqrt_minus_1 << "^2 = "
              << (sqrt_minus_1 * sqrt_minus_1) % p << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - 65537 ay Fermat prime na may\n";
    std::cout << "    order 2^16 na cyclic group\n";
    std::cout << "  - Ang 0 ay walang discrete log,\n";
    std::cout << "    kaya ang NAND ay may special\n";
    std::cout << "    behavior sa 0\n";
    std::cout << "  - sqrt(-1) = " << sqrt_minus_1 << "\n";
    std::cout << "    ay may period 4\n";
    std::cout << "========================================\n";

    return 0;
}
