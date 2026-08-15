#include <iostream>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <vector>

constexpr long Q = 536870909;

int main() {
    std::cout << "φ-BASED KEY GENERATION\n";
    std::cout << "======================\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Na-verify na natin:
    // φ = 386640388 mod Q
    // ψ = 150230522 mod Q
    // φ² = φ + 1
    // φ·ψ = -1
    
    long phi_val = 386640388;
    long psi_val = 150230522;
    
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_val);
    NTL::ZZ_p psi = NTL::to_ZZ_p(psi_val);
    
    // IDEA: Gamitin ang φ para sa relinearization key
    // Sa halip na random na encrypted s², gumamit ng φ-structured key
    
    std::cout << "1. φ-STRUCTURED RELINEARIZATION KEY\n";
    std::cout << "   K = Encrypt(s² mod φ)\n";
    std::cout << "   Kung s² ≡ s + 1 (mod φ), automatic reduction\n\n";
    
    // Check: s² ≡ s + 1 sa evaluation sa φ
    // Kung ang s(x) ay may s(φ) = φ, then s²(φ) = φ² = φ + 1
    
    std::cout << "2. EVALUATION POINT METHOD\n";
    std::cout << "   I-evaluate ang polynomials sa x = φ\n";
    std::cout << "   Doon, automatic ang s² = s + 1\n\n";
    
    // Test: Fibonacci identity sa Z_Q
    // F(n) = (φ^n - ψ^n) / √5 mod Q
    long sqrt5 = 236409866;  // Na-compute na natin
    
    std::cout << "3. FIBONACCI SA Z_Q\n";
    std::cout << "   √5 mod Q = " << sqrt5 << "\n\n";
    
    // Compute F(10) gamit ang Binet formula
    NTL::ZZ_p phi_n = NTL::power(phi, 10);
    NTL::ZZ_p psi_n = NTL::power(psi, 10);
    NTL::ZZ_p sqrt5_zz = NTL::to_ZZ_p(sqrt5);
    
    NTL::ZZ_p fib_10 = (phi_n - psi_n) / sqrt5_zz;
    std::cout << "   F(10) via Binet: " << fib_10 << "\n";
    std::cout << "   Expected F(10) = 55\n\n";
    
    // 4. PHI AS MODULUS REDUCTION
    std::cout << "4. φ-BASED MODULUS REDUCTION\n";
    std::cout << "   Sa halip na X^1024 + 1, gamitin ang φ para\n";
    std::cout << "   sa coefficient reduction\n\n";
    
    // Test: Paano mag-reduce ng coefficients gamit ang φ
    std::cout << "   Coefficient reduction:\n";
    std::cout << "   c mod φ = c - ⌊c/φ⌋ · φ\n";
    std::cout << "   Ito ay nagbibigay ng smaller coefficients\n\n";
    
    // 5. KEY INSIGHT
    std::cout << "5. KEY INSIGHT\n";
    std::cout << "   Ang φ ay nasa Z_Q, kaya:\n";
    std::cout << "   - Pwede sa coefficient modulus\n";
    std::cout << "   - Pwede sa evaluation\n";
    std::cout << "   - Pwede sa key generation\n";
    std::cout << "   - HINDI pwede sa polynomial ring (X^1024 + 1)\n\n";
    
    std::cout << "ANO ANG PWEDE GAWIN:\n";
    std::cout << "a. Gumamit ng φ-scaled coefficients sa secret key\n";
    std::cout << "b. I-evaluate sa φ para sa bootstrapping\n";
    std::cout << "c. φ-based noise threshold (ginagawa mo na)\n";
    std::cout << "d. Fibonacci-based key distribution\n";
    
    return 0;
}
