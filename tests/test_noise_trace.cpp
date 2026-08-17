// TRACE ANG NOISE SA BAWAT NAND
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NOISE TRACE PER NAND\n";
    std::cout << "====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto current = ct1;
    
    std::cout << "Depth | Noise v | Decrypt | Expected\n";
    std::cout << "------|---------|---------|---------\n";
    
    for (int i = 0; i <= 10; i++) {
        NTL::ZZ_pX noise = current.first + current.second * fhe.s;
        fhe.reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        bool dec = fhe.decrypt(current);
        bool exp = (i % 2 == 0) ? 1 : 0;  // Start sa 1, then 0, 1, 0...
        
        std::cout << i << " | " << v << " | " << dec << " | " << exp << "\n";
        
        // Next depth
        current = fhe.nand_gate(current, current);
    }
    
    return 0;
}
