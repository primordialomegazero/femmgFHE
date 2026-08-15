#include "../src/fhe/golden_gsw_fhe.h"
#include <iostream>

int main() {
    GoldenGSW::init_ring();
    
    // Test reduce_mod sa simple polynomial
    NTL::ZZ_pX poly;
    NTL::SetCoeff(poly, 0, 1);
    NTL::SetCoeff(poly, 1024, 5);  // Dapat ma-reduce sa -5 sa coefficient 0
    NTL::SetCoeff(poly, 1025, 3);  // Dapat ma-reduce sa -3 sa coefficient 1
    
    std::cout << "Before reduce: deg = " << NTL::deg(poly) << "\n";
    GoldenGSW::reduce_mod(poly);
    std::cout << "After reduce: deg = " << NTL::deg(poly) << "\n";
    std::cout << "coeff[0] = " << NTL::coeff(poly, 0) << " (expected 1-5 = -4)\n";
    std::cout << "coeff[1] = " << NTL::coeff(poly, 1) << " (expected 3-3 = 0)\n";
    
    // Test sa multiply result
    GoldenGSW::PublicKey pk;
    GoldenGSW::SecretKey sk;
    GoldenGSW::keygen(pk, sk, 42);
    
    auto ct0 = GoldenGSW::encrypt_gsw(pk, false, 1000);
    auto ct1 = GoldenGSW::encrypt_gsw(pk, true, 2000);
    
    std::cout << "\nct0.m00 deg = " << NTL::deg(ct0.m00) << "\n";
    std::cout << "ct0.m01 deg = " << NTL::deg(ct0.m01) << "\n";
    
    auto mult = GoldenGSW::multiply_gsw(ct0, ct1);
    
    std::cout << "\nmult.m00 deg = " << NTL::deg(mult.m00) << "\n";
    std::cout << "mult.m01 deg = " << NTL::deg(mult.m01) << "\n";
    
    return 0;
}
