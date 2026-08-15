#include <iostream>
#include <iomanip>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <vector>
#include <cmath>

constexpr long Q = 536870909;

int main() {
    std::cout << "NOISE PATTERN WITH φ-SCALING\n";
    std::cout << "============================\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    long phi_val = 386640388;
    long psi_val = 150230522;
    
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_val);
    NTL::ZZ_p psi = NTL::to_ZZ_p(psi_val);
    
    // IDEA: Ang noise ay maaaring mag-follow ng φ-pattern
    // Kung ang noise ay sumusunod sa Lucas numbers, may cancellation
    
    std::cout << "Lucas numbers (φ^n + ψ^n) mod Q:\n";
    std::cout << "n\tL(n) mod Q\tGrowth\n";
    std::cout << "---\t----------\t------\n";
    
    NTL::ZZ_p prev = NTL::to_ZZ_p(2);  // L(0) = 2
    for (int n = 1; n <= 20; n++) {
        NTL::ZZ_p phi_n = NTL::power(phi, n);
        NTL::ZZ_p psi_n = NTL::power(psi, n);
        NTL::ZZ_p lucas = phi_n + psi_n;
        
        long prev_val = NTL::conv<long>(prev);
        long curr_val = NTL::conv<long>(lucas);
        
        double growth = (prev_val > 0) ? 
            (double)curr_val / (double)prev_val : 0;
        
        std::cout << n << "\t" << curr_val << "\t\t" 
                  << std::fixed << std::setprecision(4) << growth << "\n";
        
        prev = lucas;
    }
    
    std::cout << "\n=== KEY OBSERVATION ===\n\n";
    std::cout << "Ang Lucas numbers ay may property:\n";
    std::cout << "L(n+1) / L(n) → φ as n → ∞\n\n";
    
    std::cout << "Kung ang noise ay sumusunod sa Lucas pattern:\n";
    std::cout << "  e_{n+1} = e_n + e_{n-1} (Fibonacci recurrence)\n";
    std::cout << "  May cancellation kapag φ^n + ψ^n = integer\n\n";
    
    // Subukan: Noise na may φ-structured coefficients
    std::cout << "=== φ-STRUCTURED NOISE TEST ===\n\n";
    
    // Kung ang noise polynomial ay may φ sa coefficients:
    // e(x) = e0 + e1·x + e2·x² + ... 
    // Kung ei = F(i) mod Q, may pattern
    
    NTL::ZZ_pX noise;
    long f0 = 0, f1 = 1;
    for (int i = 0; i < 10; i++) {
        NTL::SetCoeff(noise, i, f1 % 10);  // Small noise
        long f2 = f0 + f1;
        f0 = f1;
        f1 = f2;
    }
    
    std::cout << "Fibonacci-based noise coefficients: ";
    f0 = 0; f1 = 1;
    for (int i = 0; i < 10; i++) {
        std::cout << (f1 % 10) << " ";
        long f2 = f0 + f1;
        f0 = f1;
        f1 = f2;
    }
    std::cout << "\n\n";
    
    // Check: May pattern ba sa decryption?
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, 1);
    NTL::SetCoeff(s, 1, 1);
    
    NTL::ZZ_pX c0 = noise;
    NTL::ZZ_pX c1 = s;
    
    // Decrypt: c0 + c1·s = message + error
    NTL::ZZ_pX decrypted = c0 + c1 * s;
    
    std::cout << "Decrypted polynomial (first 5 coefficients):\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "  [" << i << "] = " << NTL::conv<long>(NTL::coeff(decrypted, i)) << "\n";
    }
    
    std::cout << "\n=== POSSIBLE APPLICATION ===\n\n";
    std::cout << "1. Fibonacci noise coefficients → predictable cancellation\n";
    std::cout << "2. Lucas number alignment → automatic rounding\n";
    std::cout << "3. φ-scaled threshold → optimal separation\n";
    std::cout << "4. Binet formula → closed-form noise bound\n\n";
    
    std::cout << "NEXT STEP: Test kung ang Fibonacci-noise ay\n";
    std::cout << "may mas mabagal na growth kaysa random noise\n";
    
    return 0;
}
