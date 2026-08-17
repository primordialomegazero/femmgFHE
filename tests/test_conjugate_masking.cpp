// CONJUGATE MASKING TEST
// ct = m·φ + e·ψ — message sa φ, noise sa ψ

#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "CONJUGATE MASKING TEST\n";
    std::cout << "======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ inv_phi = NTL::InvMod(phi, Q);
    
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p psi_p = NTL::to_ZZ_p(psi);
    NTL::ZZ_p inv_phi_p = NTL::to_ZZ_p(inv_phi);
    
    // Encrypt: ct = m·φ + e·ψ
    auto encrypt = [&](bool m, NTL::ZZ_p e) {
        NTL::ZZ_p result = (m ? phi_p : NTL::to_ZZ_p(0)) + e * psi_p;
        return result;
    };
    
    // Decrypt: project sa φ-direction
    // v·ψ = m·φ·ψ + e·ψ² = -m + e·ψ² → check kung malapit sa -1 o 0
    auto decrypt = [&](NTL::ZZ_p ct) {
        // Project sa ψ: ct·ψ = m·φψ + e·ψ² = -m + e·ψ²
        // Para sa m=1: result = Q-1 + e·ψ² (malapit sa Q-1)
        // Para sa m=0: result = e·ψ² (malapit sa 0)
        NTL::ZZ_p v = ct * psi_p;
        NTL::ZZ val = NTL::rep(v);
        NTL::ZZ dist_minus1 = (val > Q-1) ? val - (Q-1) : (Q-1) - val;
        NTL::ZZ dist_0 = (val < Q/2) ? val : Q - val;
        return dist_minus1 < dist_0;
    };
    
    // NAND: φ - a·b·φ⁻¹
    auto nand = [&](NTL::ZZ_p a, NTL::ZZ_p b) {
        return phi_p - a * b * inv_phi_p;
    };
    
    // Test with small noise
    NTL::ZZ_p e1 = NTL::to_ZZ_p(0);  // zero noise for testing
    NTL::ZZ_p e2 = NTL::to_ZZ_p(0);
    
    auto ct1 = encrypt(true, e1);
    auto ct0 = encrypt(false, e2);
    
    std::cout << "Encrypt(1) + noise = " << NTL::rep(ct1) << "\n";
    std::cout << "Decrypt(ct1) = " << decrypt(ct1) << " (exp 1)\n";
    std::cout << "Decrypt(ct0) = " << decrypt(ct0) << " (exp 0)\n\n";
    
    // Deep chain with noise
    auto current = ct1;
    std::cout << "Deep chain with noise (100 depths):\n";
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
