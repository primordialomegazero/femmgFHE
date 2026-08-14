#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(536870909));
    
    // Test: Sa ring Z_Q[X]/(X^N + 1), N=1024
    // Ang multiplication ay modulo X^1024 + 1
    
    NTL::ZZ_pX a, b;
    NTL::SetCoeff(a, 0, 100);
    NTL::SetCoeff(a, 1, 200);
    NTL::SetCoeff(b, 0, 300);
    NTL::SetCoeff(b, 1, 400);
    
    // Direct multiplication (walang reduction)
    NTL::ZZ_pX c = a * b;
    
    std::cout << "Direct multiplication:\n";
    std::cout << "c[0] = " << NTL::conv<long>(NTL::coeff(c, 0)) << "\n";
    std::cout << "c[1] = " << NTL::conv<long>(NTL::coeff(c, 1)) << "\n";
    std::cout << "c[2] = " << NTL::conv<long>(NTL::coeff(c, 2)) << "\n";
    
    // Ang tamang multiplication sa ring ay dapat mag-reduce modulo X^1024 + 1
    // Pero para sa practical purposes, ang NTT-based multiplication ang kailangan
    
    // Para sa test: ang simpleng polynomial multiplication ay hindi sapat
    // Kailangan natin ng reduction modulo X^N + 1
    
    return 0;
}
