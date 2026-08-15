#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pXFactoring.h>
#include <iostream>

constexpr long Q = 536870909;

int main() {
    std::cout << "IRREDUCIBILITY CHECK: X^1024 - X - 1 sa Z_" << Q << "\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Gumawa ng polynomial X^1024 - X - 1
    NTL::ZZ_pX poly;
    NTL::SetCoeff(poly, 1024, 1);
    NTL::SetCoeff(poly, 1, -1);
    NTL::SetCoeff(poly, 0, -1);
    
    std::cout << "Polynomial: X^1024 - X - 1\n\n";
    
    // Check kung irreducible gamit ang ProbIrredTest
    long result = NTL::ProbIrredTest(poly);
    
    std::cout << "ProbIrredTest result: " << result << "\n";
    std::cout << "0 = reducible, 1 = irreducible\n\n";
    
    if (result == 1) {
        std::cout << "✓ IRREDUCIBLE - valid para sa field!\n";
    } else {
        std::cout << "✗ REDUCIBLE - hindi field, masisira ang RLWE\n";
    }
    
    // I-compare sa traditional X^1024 + 1
    NTL::ZZ_pX traditional;
    NTL::SetCoeff(traditional, 1024, 1);
    NTL::SetCoeff(traditional, 0, 1);
    
    long trad_result = NTL::ProbIrredTest(traditional);
    std::cout << "\nTraditional X^1024 + 1: " << trad_result << "\n";
    
    return 0;
}
