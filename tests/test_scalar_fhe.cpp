// SCALAR FHE — Pure φ, walang polynomial ring
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "SCALAR FHE TEST\n";
    std::cout << "===============\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);

    // Scalar φ (hindi polynomial)
    NTL::ZZ phi = fhe.golden_plain;
    NTL::ZZ inv_phi = fhe.inv_golden;

    // Encrypt(1) = φ, Encrypt(0) = 0
    NTL::ZZ enc1 = phi;
    NTL::ZZ enc0 = NTL::to_ZZ(0);

    std::cout << "Encrypt(1) = " << enc1 << "\n";
    std::cout << "Encrypt(0) = " << enc0 << "\n\n";

    // NAND = φ - a·b·φ⁻¹ mod Q
    auto scalar_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ result = (phi - (a * b) % Q * inv_phi) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Decrypt: 1 kung malapit sa φ, 0 kung malapit sa 0
    auto scalar_decrypt = [&](NTL::ZZ v) {
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ d_phi = (v > phi) ? v - phi : phi - v;
        NTL::ZZ d_psi = (v > fhe.psi_zz) ? v - fhe.psi_zz : fhe.psi_zz - v;
        NTL::ZZ dist_phi_orbit = (d_phi < d_psi) ? d_phi : d_psi;
        return dist_phi_orbit < dist_0;
    };

    // Deep chain
    NTL::ZZ current = enc1;
    std::cout << "Deep chain (100 depths):\n";
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = scalar_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = scalar_nand(current, current);
    }

    std::cout << "Errors: " << errors << "/101\n";
    return 0;
}
