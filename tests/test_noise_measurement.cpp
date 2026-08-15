#include "../src/fhe/golden_fibonacci_fhe.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "NOISE MEASUREMENT — 257-bit FHE\n";
    std::cout << "================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe::FibonacciFHE fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true, 2000);
    auto current = ct1;
    
    std::cout << "Depth | Noise Value (c0 coefficient) | Distance to golden_plain | Distance to 0\n";
    std::cout << "------|-------------------------------|-------------------------|------------\n";
    
    for (int depth = 0; depth <= 100; depth++) {
        // Measure noise
        NTL::ZZ_pX noise = current.first + current.second * fhe.s;
        golden_fhe::reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        NTL::ZZ dist_g = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        
        if (depth <= 10 || depth % 10 == 0) {
            std::cout << depth << " | " << v << " | " << dist_g << " | " << dist_0 << "\n";
        }
        
        // Next depth: NOT operation
        current = fhe.nand_gate(current, current);
    }
    
    return 0;
}
