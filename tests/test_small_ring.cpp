#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(536870909));
    
    // Test sa mas maliit na ring: Z_Q[X]/(X^4 + 1)
    // Dito, X^4 = -1
    
    NTL::ZZ_pX a, b;
    NTL::SetCoeff(a, 0, 100);
    NTL::SetCoeff(a, 1, 200);
    NTL::SetCoeff(b, 0, 300);
    NTL::SetCoeff(b, 1, 400);
    
    // Direct multiplication
    NTL::ZZ_pX c = a * b;
    
    std::cout << "Direct multiplication:\n";
    std::cout << "c[0] = " << NTL::conv<long>(NTL::coeff(c, 0)) << " (expected 30000)\n";
    std::cout << "c[1] = " << NTL::conv<long>(NTL::coeff(c, 1)) << " (expected 100000)\n";
    std::cout << "c[2] = " << NTL::conv<long>(NTL::coeff(c, 2)) << " (expected 80000)\n";
    
    // Sa ring Z_Q[X]/(X^4 + 1):
    // c[2]*X^2 ay dapat ma-reduce since X^4 = -1
    // Pero ang c[2]*X^2 ay hindi directly ma-reduce sa X^4 + 1
    
    // Ang tamang approach ay modulo X^4 + 1
    // X^4 = -1, kaya X^4 + 1 = 0
    
    return 0;
}
