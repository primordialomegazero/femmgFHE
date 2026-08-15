#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;

void reduce_mod(NTL::ZZ_pX& poly) {
    // Proper reduction: X^N = -1
    // Kaya X^(N + k) = -X^k
    // At X^(2N + k) = X^k
    // General: X^(mN + k) = (-1)^m · X^k
    
    if (NTL::deg(poly) < N) return;
    
    NTL::ZZ_pX result;
    result.SetLength(N);
    
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        if (coeff == 0) continue;
        
        int reduced_deg = i % (2 * N);  // Period ay 2N (kasi X^N = -1)
        int sign = 1;
        
        if (reduced_deg >= N) {
            reduced_deg -= N;
            sign = -1;
        }
        
        NTL::SetCoeff(result, reduced_deg, 
                     NTL::coeff(result, reduced_deg) + sign * coeff);
    }
    
    poly = result;
}

int main() {
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Test case
    NTL::ZZ_pX poly;
    NTL::SetCoeff(poly, 0, 1);
    NTL::SetCoeff(poly, 1024, 5);  // X^1024 = -1, dapat -5 sa coeff 0
    NTL::SetCoeff(poly, 1025, 3);  // X^1025 = -X, dapat -3 sa coeff 1
    
    std::cout << "Before reduce: deg = " << NTL::deg(poly) << "\n";
    reduce_mod(poly);
    std::cout << "After reduce: deg = " << NTL::deg(poly) << "\n";
    std::cout << "coeff[0] = " << NTL::conv<long>(NTL::coeff(poly, 0)) 
              << " (expected -4)\n";
    std::cout << "coeff[1] = " << NTL::conv<long>(NTL::coeff(poly, 1)) 
              << " (expected -3)\n\n";
    
    // Test with higher degree
    NTL::ZZ_pX poly2;
    NTL::SetCoeff(poly2, 2048, 7);  // X^2048 = 1, dapat 7 sa coeff 0
    NTL::SetCoeff(poly2, 2049, 2);  // X^2049 = X, dapat 2 sa coeff 1
    NTL::SetCoeff(poly2, 3072, 4);  // X^3072 = -1, dapat -4 sa coeff 0
    
    std::cout << "Higher degree test:\n";
    reduce_mod(poly2);
    std::cout << "coeff[0] = " << NTL::conv<long>(NTL::coeff(poly2, 0)) 
              << " (expected 7 - 4 = 3)\n";
    std::cout << "coeff[1] = " << NTL::conv<long>(NTL::coeff(poly2, 1)) 
              << " (expected 2)\n";
    
    return 0;
}
