#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto current = ct1;
    
    std::cout << "φ = " << fhe.golden_plain << "\n";
    std::cout << "ψ = " << fhe.psi_zz << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n\n";
    
    for (int i = 0; i <= 8; i++) {
        NTL::ZZ_pX noise = current.first + current.second * fhe.s;
        fhe.reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        // Compute distances
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ d_phi = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
        NTL::ZZ d_psi = (v > fhe.psi_zz) ? v - fhe.psi_zz : fhe.psi_zz - v;
        NTL::ZZ dist_phi_orbit = (d_phi < d_psi) ? d_phi : d_psi;
        
        std::cout << "NAND[" << i << "]: v=" << v << "\n";
        std::cout << "  dist_0=" << dist_0 << ", dist_φ_orbit=" << dist_phi_orbit << "\n";
        std::cout << "  Orbit: " << (dist_phi_orbit < dist_0 ? "φ-orbit" : "0-orbit") << "\n\n";
        
        current = fhe.nand_gate(current, current);
    }
    
    return 0;
}
