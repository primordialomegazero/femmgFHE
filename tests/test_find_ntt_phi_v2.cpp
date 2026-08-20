// FIND NTT-FRIENDLY φ-PRIME V2
// 1. Maghanap muna sa 32-bit range para ma-verify
// 2. Tapos sa 64-bit
// 3. Tapos sa 128-bit na may mas maraming attempts

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NTT-FRIENDLY φ-PRIME SEARCH V2\n";
    std::cout << "========================================\n\n";

    // Test sa 32-bit muna (mabilis)
    std::cout << "32-BIT SEARCH (N=1024):\n";
    NTL::ZZ Q32 = NTL::to_ZZ(1) << 31;
    NTL::ZZ lcm32 = NTL::to_ZZ(5 * 1024);
    NTL::ZZ start32 = Q32 - (Q32 % lcm32) + lcm32;

    int found32 = 0;
    for (NTL::ZZ Q = start32; found32 < 3 && NTL::NumBits(Q) < 33; Q += lcm32) {
        if (NTL::ProbPrime(Q, 15) == 1) {
            found32++;
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
            std::cout << "  Q = " << Q << " (bits=" << NTL::NumBits(Q) << ")\n";
            std::cout << "    Q mod 5 = " << Q % 5 << "\n";
            std::cout << "    Q mod 1024 = " << Q % 1024 << "\n";
            std::cout << "    √5 = " << sqrt5 << "\n\n";
        }
    }

    // Test sa 64-bit
    std::cout << "64-BIT SEARCH (N=1024):\n";
    NTL::ZZ Q64 = NTL::to_ZZ(1) << 63;
    NTL::ZZ start64 = Q64 - (Q64 % lcm32) + lcm32;

    int found64 = 0;
    int attempts64 = 0;
    for (NTL::ZZ Q = start64; found64 < 3 && attempts64 < 10000; Q += lcm32) {
        attempts64++;
        if (NTL::ProbPrime(Q, 15) == 1) {
            found64++;
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
            std::cout << "  Q = " << Q << " (bits=" << NTL::NumBits(Q) << ")\n";
            std::cout << "    Q mod 5 = " << Q % 5 << "\n";
            std::cout << "    Q mod 1024 = " << Q % 1024 << "\n";
            std::cout << "    √5 = " << sqrt5 << "\n\n";
        }
    }

    std::cout << "  64-bit attempts: " << attempts64 << "\n";
    std::cout << "  Found: " << found64 << "\n\n";

    // I-extrapolate sa 128-bit
    // Prime density sa 64-bit: ~1/ln(2^64) ≈ 1/44
    // Prime density sa 128-bit: ~1/ln(2^128) ≈ 1/89
    // Kaya para sa 128-bit, kailangan ~2x attempts
    // Sa 10000 attempts, dapat makakahanap ng ~100+ primes

    std::cout << "128-BIT SEARCH (N=1024, 20000 attempts):\n";
    NTL::ZZ Q128 = NTL::to_ZZ(1) << 127;
    NTL::ZZ start128 = Q128 - (Q128 % lcm32) + lcm32;

    int found128 = 0;
    int attempts128 = 0;
    for (NTL::ZZ Q = start128; found128 < 3 && attempts128 < 20000; Q += lcm32) {
        attempts128++;
        if (NTL::ProbPrime(Q, 20) == 1) {
            found128++;
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
            std::cout << "  Q = " << Q << " (bits=" << NTL::NumBits(Q) << ")\n";
            std::cout << "    √5 = " << sqrt5 << "\n\n";
        }
    }

    std::cout << "  128-bit attempts: " << attempts128 << "\n";
    std::cout << "  Found: " << found128 << "\n";

    return 0;
}
