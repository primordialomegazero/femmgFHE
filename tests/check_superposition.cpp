#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "SUPERPOSITION CHECK - LAHAT NG GATES\n\n";
    
    // Basic gates
    auto nand00 = fhe.nand_gate(ct0, ct0);
    auto nand01 = fhe.nand_gate(ct0, ct1);
    auto nand10 = fhe.nand_gate(ct1, ct0);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    std::cout << "NAND(0,0) = " << fhe.decrypt(nand00) << " exp 1\n";
    std::cout << "NAND(0,1) = " << fhe.decrypt(nand01) << " exp 1\n";
    std::cout << "NAND(1,0) = " << fhe.decrypt(nand10) << " exp 1\n";
    std::cout << "NAND(1,1) = " << fhe.decrypt(nand11) << " exp 0\n\n";
    
    // Derived gates
    auto not0 = fhe.not_gate(ct0);
    auto not1 = fhe.not_gate(ct1);
    std::cout << "NOT(0) = " << fhe.decrypt(not0) << " exp 1\n";
    std::cout << "NOT(1) = " << fhe.decrypt(not1) << " exp 0\n\n";
    
    auto and11 = fhe.and_gate(ct1, ct1);
    std::cout << "AND(1,1) = " << fhe.decrypt(and11) << " exp 1\n\n";
    
    auto or00 = fhe.or_gate(ct0, ct0);
    std::cout << "OR(0,0) = " << fhe.decrypt(or00) << " exp 0\n\n";
    
    auto xor01 = fhe.xor_gate(ct0, ct1);
    std::cout << "XOR(0,1) = " << fhe.decrypt(xor01) << " exp 1\n\n";
    
    auto cnot10 = fhe.cnot(ct1, ct0);
    std::cout << "CNOT(1,0) = " << fhe.decrypt(cnot10) << " exp 1\n\n";
    
    // I-check kung may superposition (parehong malapit sa 0 at φ)
    auto check_orbit = [&](const char* name, const golden_fhe_v6::FibonacciFHEV6::Cipher& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * fhe.s;
        fhe.reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
        NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
        std::cout << name << ": v=" << v 
                  << " dist_0=" << dist_0 
                  << " dist_g=" << dist_golden << "\n";
    };
    
    std::cout << "\n=== ORBIT ANALYSIS ===\n";
    check_orbit("NAND(0,0)", nand00);
    check_orbit("NAND(1,1)", nand11);
    check_orbit("NOT(0)", not0);
    check_orbit("NOT(1)", not1);
    check_orbit("AND(1,1)", and11);
    check_orbit("OR(0,0)", or00);
    check_orbit("XOR(0,1)", xor01);
    check_orbit("CNOT(1,0)", cnot10);
    
    return 0;
}
