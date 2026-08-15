#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pXFactoring.h>
#include <iostream>
#include <vector>

constexpr long Q = 536870909;

int main() {
    std::cout << "HANAP: IRREDUCIBLE POLYNOMIAL NA MAY φ-PROPERTY\n";
    std::cout << "================================================\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Test iba't ibang polynomial na may φ-structure
    std::vector<std::pair<std::string, NTL::ZZ_pX>> candidates;
    
    // X² - X - 1 (classic φ polynomial)
    NTL::ZZ_pX p1;
    NTL::SetCoeff(p1, 2, 1);
    NTL::SetCoeff(p1, 1, -1);
    NTL::SetCoeff(p1, 0, -1);
    candidates.push_back({"X² - X - 1", p1});
    
    // X² + X - 1 (conjugate φ polynomial)
    NTL::ZZ_pX p2;
    NTL::SetCoeff(p2, 2, 1);
    NTL::SetCoeff(p2, 1, 1);
    NTL::SetCoeff(p2, 0, -1);
    candidates.push_back({"X² + X - 1", p2});
    
    // X⁴ - X² - 1 (degree 4 φ extension)
    NTL::ZZ_pX p3;
    NTL::SetCoeff(p3, 4, 1);
    NTL::SetCoeff(p3, 2, -1);
    NTL::SetCoeff(p3, 0, -1);
    candidates.push_back({"X⁴ - X² - 1", p3});
    
    // X⁸ - X⁴ - 1 (degree 8 φ extension)
    NTL::ZZ_pX p4;
    NTL::SetCoeff(p4, 8, 1);
    NTL::SetCoeff(p4, 4, -1);
    NTL::SetCoeff(p4, 0, -1);
    candidates.push_back({"X⁸ - X⁴ - 1", p4});
    
    // X^16 - X^8 - 1 (degree 16 φ extension)
    NTL::ZZ_pX p5;
    NTL::SetCoeff(p5, 16, 1);
    NTL::SetCoeff(p5, 8, -1);
    NTL::SetCoeff(p5, 0, -1);
    candidates.push_back({"X^16 - X^8 - 1", p5});
    
    // X^32 - X^16 - 1 (degree 32 φ extension)
    NTL::ZZ_pX p6;
    NTL::SetCoeff(p6, 32, 1);
    NTL::SetCoeff(p6, 16, -1);
    NTL::SetCoeff(p6, 0, -1);
    candidates.push_back({"X^32 - X^16 - 1", p6});
    
    // X^N - X^(N/2) - 1 para sa powers of 2
    for (int N = 64; N <= 2048; N *= 2) {
        NTL::ZZ_pX p;
        NTL::SetCoeff(p, N, 1);
        NTL::SetCoeff(p, N/2, -1);
        NTL::SetCoeff(p, 0, -1);
        candidates.push_back({"X^" + std::to_string(N) + " - X^" + 
                            std::to_string(N/2) + " - 1", p});
    }
    
    std::cout << "Polynomial\t\t\tIrreducible?\n";
    std::cout << "-----------\t\t\t------------\n";
    
    for (const auto& cand : candidates) {
        long result = NTL::ProbIrredTest(cand.second);
        
        std::cout << cand.first << "\t\t" 
                  << (result == 1 ? "✓ YES" : "✗ NO") << "\n";
    }
    
    // Subukan ang cyclotomic polynomials na may φ-property
    std::cout << "\n=== CYCLOTOMIC POLYNOMIALS ===\n\n";
    
    // Φ_5(X) = X⁴ + X³ + X² + X + 1
    NTL::ZZ_pX cyclotomic5;
    for (int i = 0; i <= 4; i++) {
        NTL::SetCoeff(cyclotomic5, i, 1);
    }
    long result5 = NTL::ProbIrredTest(cyclotomic5);
    std::cout << "Φ_5(X) = X⁴+X³+X²+X+1: " 
              << (result5 == 1 ? "IRREDUCIBLE" : "REDUCIBLE") << "\n";
    
    // Φ_10(X) = X⁴ - X³ + X² - X + 1
    NTL::ZZ_pX cyclotomic10;
    NTL::SetCoeff(cyclotomic10, 4, 1);
    NTL::SetCoeff(cyclotomic10, 3, -1);
    NTL::SetCoeff(cyclotomic10, 2, 1);
    NTL::SetCoeff(cyclotomic10, 1, -1);
    NTL::SetCoeff(cyclotomic10, 0, 1);
    long result10 = NTL::ProbIrredTest(cyclotomic10);
    std::cout << "Φ_10(X) = X⁴-X³+X²-X+1: " 
              << (result10 == 1 ? "IRREDUCIBLE" : "REDUCIBLE") << "\n";
    
    // Φ_15(X) = X⁸ - X⁷ + X⁵ - X⁴ + X³ - X + 1
    NTL::ZZ_pX cyclotomic15;
    NTL::SetCoeff(cyclotomic15, 8, 1);
    NTL::SetCoeff(cyclotomic15, 7, -1);
    NTL::SetCoeff(cyclotomic15, 5, 1);
    NTL::SetCoeff(cyclotomic15, 4, -1);
    NTL::SetCoeff(cyclotomic15, 3, 1);
    NTL::SetCoeff(cyclotomic15, 1, -1);
    NTL::SetCoeff(cyclotomic15, 0, 1);
    long result15 = NTL::ProbIrredTest(cyclotomic15);
    std::cout << "Φ_15(X) = X⁸-X⁷+X⁵-X⁴+X³-X+1: " 
              << (result15 == 1 ? "IRREDUCIBLE" : "REDUCIBLE") << "\n";
    
    return 0;
}
