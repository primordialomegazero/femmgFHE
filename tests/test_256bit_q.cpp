#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    std::cout << "256-BIT Q SEARCH\n\n";
    
    // Maghanap ng 256-bit prime na Q ≡ 1 o 4 mod 5
    // Kailangan: Q ay prime, Q ≡ 1 mod 5 (para sa φ)
    
    // 2^256 - 189 = karaniwang prime sa crypto (secp256k1 prime)
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129639747");
    
    std::cout << "Starting Q: " << Q << "\n";
    std::cout << "Bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Mod 5: " << Q % 5 << "\n\n";
    
    // I-adjust para Q ≡ 1 mod 5
    NTL::ZZ adjustment = NTL::to_ZZ(1);
    adjustment = (adjustment - (Q % 5) + 5) % 5;
    Q += adjustment;
    
    std::cout << "Adjusted Q: " << Q << "\n";
    std::cout << "Mod 5: " << Q % 5 << "\n\n";
    
    // Hanapin ang prime na Q ≡ 1 mod 5
    int attempts = 0;
    while (NTL::ProbPrime(Q, 40) == 0 && attempts < 1000) {
        Q += 5;
        attempts++;
    }
    
    if (NTL::ProbPrime(Q, 40)) {
        std::cout << "FOUND PRIME: " << Q << "\n";
        std::cout << "Bits: " << NTL::NumBits(Q) << "\n";
        std::cout << "Mod 5: " << Q % 5 << "\n";
        std::cout << "Attempts: " << attempts << "\n\n";
        
        // Compute φ
        NTL::ZZ_p::init(Q);
        NTL::ZZ five = NTL::to_ZZ("5");
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, five % Q, Q);
        
        if (sqrt5 != 0) {
            NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ("2"), Q);
            NTL::ZZ phi = ((NTL::to_ZZ("1") + sqrt5) * inv2) % Q;
            
            NTL::ZZ phi_sq = (phi * phi) % Q;
            NTL::ZZ phi_plus_1 = (phi + 1) % Q;
            
            std::cout << "φ = " << phi << "\n";
            std::cout << "φ² = φ+1: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
            
            // Key space
            std::cout << "=== KEY SPACE ===\n";
            std::cout << "Bits: " << NTL::NumBits(Q) << "\n";
            std::cout << "Key space: 2^" << NTL::NumBits(Q) << "\n";
            std::cout << "≈ " << std::scientific 
                      << std::pow(2.0, NTL::NumBits(Q)) << " possibilities\n";
            std::cout << "Brute force: IMPOSSIBLE (mas malaki sa atoms sa universe)\n";
        }
    } else {
        std::cout << "Hindi nahanap sa " << attempts << " attempts\n";
    }
    
    return 0;
}
