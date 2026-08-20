// FIND NTT-FRIENDLY PRIME NA MAY φ-STRUCTURE
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NTT-FRIENDLY φ-PRIME SEARCH\n";
    std::cout << "========================================\n\n";

    std::vector<int> N_values = {1024, 2048, 4096, 8192};

    for (int N : N_values) {
        std::cout << "N = " << N << ":\n";

        NTL::ZZ min_Q = NTL::to_ZZ(1) << 127;
        NTL::ZZ max_Q = NTL::to_ZZ(1) << 128;
        NTL::ZZ lcm_5N = NTL::to_ZZ(5 * N);

        NTL::ZZ start = min_Q - (min_Q % lcm_5N) + lcm_5N;
        if (start < min_Q) start += lcm_5N;

        int found = 0;
        int attempts = 0;

        for (NTL::ZZ Q = start; Q < max_Q && found < 3 && attempts < 500; Q += lcm_5N) {
            attempts++;
            if (NTL::ProbPrime(Q, 20) == 1) {
                found++;
                std::cout << "  FOUND: Q = " << Q << "\n";
                std::cout << "    Bits: " << NTL::NumBits(Q) << "\n";

                // Verify √5
                NTL::ZZ sqrt5;
                NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);

                // Verify √5 is correct
                NTL::ZZ check = (sqrt5 * sqrt5) % Q;
                std::cout << "    √5 mod Q: " << sqrt5 << "\n";
                std::cout << "    (√5)² mod Q = " << check << " (dapat 5)\n\n";
            }
        }

        if (found == 0) {
            std::cout << "  Walang nahanap\n\n";
        }
    }

    return 0;
}
