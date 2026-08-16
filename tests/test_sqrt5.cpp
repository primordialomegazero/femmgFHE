#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    
    NTL::ZZ sqrt5_sq = (sqrt5 * sqrt5) % Q;
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "sqrt5 = " << sqrt5 << "\n";
    std::cout << "sqrt5² mod Q = " << sqrt5_sq << " (should be 5)\n";
    
    // Compute φ manually
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ phi_sq = (phi * phi) % Q;
    NTL::ZZ phi_plus_1 = (phi + 1) % Q;
    
    std::cout << "phi = " << phi << "\n";
    std::cout << "phi² mod Q = " << phi_sq << "\n";
    std::cout << "phi+1 mod Q = " << phi_plus_1 << "\n";
    std::cout << "phi² == phi+1: " << (phi_sq == phi_plus_1 ? "YES" : "NO") << "\n";
    
    return 0;
}
