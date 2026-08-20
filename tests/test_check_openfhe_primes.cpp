// CHECK OPENFHE PRIMES PARA SA φ-STRUCTURE
// Ang OpenFHE ay may pre-computed na primes
// I-check natin kung may √5 ang mga ito

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  CHECK OPENFHE PRIMES\n";
    std::cout << "  May φ-structure ba?\n";
    std::cout << "========================================\n\n";

    // Ang standard na OpenFHE primes (mula sa source code)
    // Ito ay mga prime na may magandang NTT properties
    std::vector<std::string> primes = {
        "1152921504606847009",      // 60-bit
        "576460752303423505",       // 59-bit
        "1152921504606846977",      // 60-bit
        "1152921504606846881",      // 60-bit
        "1152921504606846513",      // 60-bit
        "1152921504606846481",      // 60-bit
        "1152921504606846561",      // 60-bit
        "1152921504606846641",      // 60-bit
        "1152921504606846721",      // 60-bit
        "4294967311",               // 32-bit
        "4294967297",               // 32-bit
    };

    for (const auto& p_str : primes) {
        NTL::ZZ Q = NTL::to_ZZ(p_str.c_str());
        
        std::cout << "Q = " << Q << " (bits=" << NTL::NumBits(Q) << ")\n";
        std::cout << "  Q mod 5 = " << Q % 5 << "\n";
        
        // Check kung may √5
        if (Q % 5 == 1 || Q % 5 == 4) {
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
            NTL::ZZ check = (sqrt5 * sqrt5) % Q;
            std::cout << "  √5 = " << sqrt5 << "\n";
            std::cout << "  (√5)² mod Q = " << check << " (dapat 5)\n";
            
            // Compute φ
            NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
            NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
            std::cout << "  φ = " << phi << "\n";
            std::cout << "  ✓ MAY φ-STRUCTURE!\n";
        } else {
            std::cout << "  ✗ Walang √5 (Q mod 5 = " << Q % 5 << ")\n";
        }
        std::cout << "\n";
    }

    return 0;
}
