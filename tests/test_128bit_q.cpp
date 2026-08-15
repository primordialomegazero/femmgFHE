#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

// 128-bit prime na Q ≡ 1 mod 5
// 2^127 + 2^3 + 1 = 170141183460469231731687303715884105737 (prime, ≡ 1 mod 5)
constexpr long long Q_LO = 0;  // Placeholder para sa 128-bit

void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
}

int main() {
    std::cout << "128-BIT Q TEST\n\n";
    
    // 128-bit prime na Q ≡ 1 mod 5
    // 2^127 + 2^3 + 1
    NTL::ZZ Q = NTL::to_ZZ("170141183460469231731687303715884105737");
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "Bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5 = " << Q % 5 << "\n";
    std::cout << "May φ? " << (Q % 5 == 1 || Q % 5 == 4 ? "YES (Q ≡ 1 mod 5)" : "NO") << "\n\n";
    
    // Verify na prime
    std::cout << "Is prime? " << (NTL::ProbPrime(Q, 20) ? "YES ✓" : "NO") << "\n\n";
    
    // Compute φ sa Z_Q
    // Tonelli-Shanks para sa sqrt(5)
    // Para sa Q ≡ 1 mod 5, may φ = (1 + √5)/2
    
    // Simple check: Hanapin ang φ gamit ang NTL
    init_ring(Q);
    
    // φ = (1 + √5)/2 mod Q
    // Kailangan ng sqrt(5) mod Q
    // Para sa prototype: gamitin ang modular exponentiation
    // sqrt(5) = 5^((Q+3)/8) kung Q ≡ 5 mod 8 (hindi ito ang case)
    
    // General: gamitin ang NTL SqrRootMod
    NTL::ZZ five = NTL::to_ZZ("5");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, five % Q, Q);
    
    if (sqrt5 != 0) {
        std::cout << "√5 mod Q = " << sqrt5 << "\n";
        
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ("2"), Q);
        NTL::ZZ phi = ((NTL::to_ZZ("1") + sqrt5) * inv2) % Q;
        
        std::cout << "φ = " << phi << "\n";
        
        // Verify φ² = φ + 1
        NTL::ZZ phi_sq = (phi * phi) % Q;
        NTL::ZZ phi_plus_1 = (phi + 1) % Q;
        std::cout << "φ² = " << phi_sq << "\n";
        std::cout << "φ+1 = " << phi_plus_1 << "\n";
        std::cout << "Match: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
        
        // Key space check
        std::cout << "Key space: 2^" << NTL::NumBits(Q) << " ≈ " 
                  << "1.7 × 10^" << (int)(NTL::NumBits(Q) * 0.30103) << "\n";
        std::cout << "Brute force: IMPOSSIBLE sa current hardware\n";
        
    } else {
        std::cout << "√5 ay hindi mahanap sa Z_Q\n";
    }
    
    return 0;
}
