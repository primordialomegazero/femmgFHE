#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ beta = Q - 1;
    std::cout << "beta (ZZ) = " << beta << "\n";
    std::cout << "beta bits: " << NTL::NumBits(beta) << "\n";
    
    NTL::ZZ_p beta_p = NTL::to_ZZ_p(beta);
    std::cout << "beta_p = " << beta_p << "\n";
    
    // Check kung ang beta_p ay ma-recover nang tama
    NTL::ZZ recovered = NTL::rep(beta_p);
    std::cout << "recovered = " << recovered << "\n";
    std::cout << "Match: " << (recovered == beta ? "YES" : "NO") << "\n";
    
    return 0;
}
