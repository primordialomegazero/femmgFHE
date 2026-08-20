// FIND NTT-FRIENDLY φ-PRIME V3
// Mas malawak na search + iba't ibang N values
// Sa OpenFHE, ang standard na primes ay may specific properties

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NTT-FRIENDLY φ-PRIME V3\n";
    std::cout << "  Mas Malawak na Search\n";
    std::cout << "========================================\n\n";

    // Para sa NTT na may ring dimension N, kailangan:
    // Q ≡ 1 (mod 2N) — para sa primitive 2N-th root of unity
    // Q ≡ 1 (mod 5)  — para sa √5
    
    // Kaya: Q ≡ 1 (mod lcm(2N, 5)) = 1 (mod 10N) para sa odd na 5

    std::vector<int> N_values = {512, 1024, 2048};
    
    for (int N : N_values) {
        int m = 10 * N;  // lcm(2N, 5) = 10N
        std::cout << "N=" << N << ", modulo " << m << ":\n";
        
        // 128-bit search
        NTL::ZZ Q = NTL::to_ZZ(1) << 127;
        NTL::ZZ start = Q - (Q % m) + m;
        
        int found = 0;
        int attempts = 0;
        int max_attempts = 50000;
        
        for (NTL::ZZ candidate = start; found < 2 && attempts < max_attempts; candidate += m) {
            attempts++;
            if (NTL::ProbPrime(candidate, 20) == 1) {
                found++;
                NTL::ZZ sqrt5;
                NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), candidate);
                NTL::ZZ check = (sqrt5 * sqrt5) % candidate;
                
                std::cout << "  Q = " << candidate << "\n";
                std::cout << "    bits = " << NTL::NumBits(candidate) << "\n";
                std::cout << "    √5 = " << sqrt5 << "\n";
                std::cout << "    check = " << check << " (dapat 5)\n\n";
            }
        }
        
        if (found == 0) {
            std::cout << "  Walang nahanap sa " << attempts << " attempts\n\n";
        }
    }

    // Alternative: Subukan ang mas maliit na primes (64-bit)
    std::cout << "64-BIT SEARCH:\n";
    for (int N : {512, 1024}) {
        int m = 10 * N;
        NTL::ZZ Q = NTL::to_ZZ(1) << 63;
        NTL::ZZ start = Q - (Q % m) + m;
        
        int found = 0;
        for (NTL::ZZ candidate = start; found < 2 && NTL::NumBits(candidate) < 65; candidate += m) {
            if (NTL::ProbPrime(candidate, 15) == 1) {
                found++;
                std::cout << "  N=" << N << ": Q = " << candidate 
                          << " (bits=" << NTL::NumBits(candidate) << ")\n";
            }
        }
        if (found == 0) {
            std::cout << "  N=" << N << ": walang nahanap\n";
        }
    }

    return 0;
}
