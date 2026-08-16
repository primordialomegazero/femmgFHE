#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    auto nand00 = fhe.nand_gate(ct0, ct0);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    // Kunin ang v values
    NTL::ZZ_pX n00 = nand00.first + nand00.second * fhe.s;
    fhe.reduce_mod(n00);
    NTL::ZZ v00 = NTL::rep(NTL::coeff(n00, 0));
    
    NTL::ZZ_pX n11 = nand11.first + nand11.second * fhe.s;
    fhe.reduce_mod(n11);
    NTL::ZZ v11 = NTL::rep(NTL::coeff(n11, 0));
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "φ = " << fhe.golden_plain << "\n";
    std::cout << "ψ = " << fhe.psi_zz << "\n\n";
    
    std::cout << "NAND(0,0) v = " << v00 << "\n";
    std::cout << "NAND(1,1) v = " << v11 << "\n\n";
    
    // Relative distance check
    // Ang 0 orbit ay {0, Q-0}
    // Ang φ orbit ay {φ, ψ}
    // Kailangan: aling orbit ang mas malapit?
    
    auto orbit_distance = [&](NTL::ZZ v) {
        NTL::ZZ dist_0_orbit = (v < Q - v) ? v : Q - v;
        NTL::ZZ d_phi = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
        NTL::ZZ d_psi = (v > fhe.psi_zz) ? v - fhe.psi_zz : fhe.psi_zz - v;
        NTL::ZZ dist_phi_orbit = (d_phi < d_psi) ? d_phi : d_psi;
        return std::make_pair(dist_0_orbit, dist_phi_orbit);
    };
    
    auto d00 = orbit_distance(v00);
    auto d11 = orbit_distance(v11);
    
    std::cout << "NAND(0,0): dist_0=" << d00.first << ", dist_φ=" << d00.second << "\n";
    std::cout << "  Result: " << (d00.second < d00.first ? 1 : 0) << " (exp 1)\n\n";
    std::cout << "NAND(1,1): dist_0=" << d11.first << ", dist_φ=" << d11.second << "\n";
    std::cout << "  Result: " << (d11.second < d11.first ? 1 : 0) << " (exp 0)\n";
    
    return 0;
}
