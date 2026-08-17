// CHECK: EXACT ba ang relinearization?
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "EXACT RELINEARIZATION CHECK\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // Check: s² - α·s - β = 0?
    NTL::ZZ s_sq = (fhe.s_val * fhe.s_val) % Q;
    NTL::ZZ alpha_s = (fhe.alpha * fhe.s_val) % Q;
    NTL::ZZ residual = (s_sq - alpha_s - fhe.beta) % Q;
    if (residual < 0) residual += Q;
    
    std::cout << "s² = " << s_sq << "\n";
    std::cout << "α·s = " << alpha_s << "\n";
    std::cout << "β = " << fhe.beta << "\n";
    std::cout << "s² - α·s - β = " << residual << "\n";
    std::cout << "EXACT: " << (residual == 0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Kung hindi exact, ano ang residual?
    if (residual != 0) {
        std::cout << "RESIDUAL ANALYSIS:\n";
        std::cout << "  Residual bits: " << NTL::NumBits(residual) << "\n";
        std::cout << "  Q bits: " << NTL::NumBits(Q) << "\n";
        std::cout << "  Ratio: " << (double)NTL::NumBits(residual) / NTL::NumBits(Q) << "\n";
        std::cout << "  → Residual ay " << NTL::NumBits(residual) << " bits out of " 
                  << NTL::NumBits(Q) << " bits Q\n";
    }
    
    // Check α and β derivation
    std::cout << "\nα = L(42) = " << fhe.alpha << "\n";
    std::cout << "F(42) = 267914296\n";
    std::cout << "F(41) = 165580141\n";
    std::cout << "L(42) = F(41) + F(43) = " << 165580141 + 433494437 << "\n\n";
    
    return 0;
}
