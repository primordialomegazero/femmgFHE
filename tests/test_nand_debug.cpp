#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NAND DEBUG - V6\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Test lahat ng NAND cases
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " (exp 0)\n";
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " (exp 1)\n";
    
    // Tingnan ang actual decryption values
    auto nand11 = fhe.nand_gate(ct1, ct1);
    NTL::ZZ_pX noise = nand11.first + nand11.second * fhe.s;
    NTL::ZZ v_zz = NTL::rep(NTL::coeff(noise, 0));
    long v = NTL::conv<long>(v_zz);
    
    NTL::ZZ Q_half = fhe.Q / 2;
    long dist_0 = (v_zz < Q_half) ? v : NTL::conv<long>(fhe.Q - v_zz);
    NTL::ZZ diff_zz = (v_zz > fhe.golden_plain) ? v_zz - fhe.golden_plain : fhe.golden_plain - v_zz;
    long diff = NTL::conv<long>(diff_zz);
    long dist_golden = (diff_zz < Q_half) ? diff : NTL::conv<long>(fhe.Q - diff_zz);
    
    std::cout << "\nNAND(1,1) raw values:\n";
    std::cout << "  v = " << v << "\n";
    std::cout << "  dist_0 = " << dist_0 << "\n";
    std::cout << "  dist_golden = " << dist_golden << "\n";
    std::cout << "  golden_plain = " << NTL::conv<long>(fhe.golden_plain) << "\n";
    std::cout << "  Q/2 = " << NTL::conv<long>(Q_half) << "\n";
    std::cout << "  Decision: " << (dist_golden < dist_0 ? "1" : "0") << "\n";
    
    return 0;
}
