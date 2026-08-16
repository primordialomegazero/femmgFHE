#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NAND DEBUG - V6 (ZZ version)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " (exp 0)\n";
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " (exp 1)\n";
    
    // Test with smaller Q para makita ang values properly
    NTL::ZZ Q_small = NTL::to_ZZ("340282366920938463463374607431768211297"); // 128-bit
    golden_fhe_v6::FibonacciFHEV6 fhe2(Q_small, 42);
    
    auto c0 = fhe2.encrypt(false);
    auto c1 = fhe2.encrypt(true);
    
    auto nand00 = fhe2.nand_gate(c0, c0);
    auto nand11 = fhe2.nand_gate(c1, c1);
    
    std::cout << "\nSmall Q test:\n";
    std::cout << "  NAND(0,0): " << fhe2.decrypt(nand00) << " (exp 1)\n";
    std::cout << "  NAND(1,1): " << fhe2.decrypt(nand11) << " (exp 0)\n";
    
    // Check raw values for NAND(1,1)
    NTL::ZZ_pX noise11 = nand11.first + nand11.second * fhe2.s;
    fhe2.reduce_mod(noise11);
    NTL::ZZ v11 = NTL::rep(NTL::coeff(noise11, 0));
    
    NTL::ZZ dist_0 = (v11 < Q_small/2) ? v11 : Q_small - v11;
    NTL::ZZ diff = (v11 > fhe2.golden_plain) ? v11 - fhe2.golden_plain : fhe2.golden_plain - v11;
    NTL::ZZ dist_golden = (diff < Q_small/2) ? diff : Q_small - diff;
    
    std::cout << "\n  NAND(1,1) values:\n";
    std::cout << "  v = " << v11 << "\n";
    std::cout << "  dist_0 = " << dist_0 << "\n";
    std::cout << "  dist_golden = " << dist_golden << "\n";
    std::cout << "  golden_plain = " << fhe2.golden_plain << "\n";
    std::cout << "  Q/2 = " << Q_small/2 << "\n";
    
    return 0;
}
