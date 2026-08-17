// BALANCED MASKING
// ct = m·φ^k + e·ψ^k + r·Q — e ay maliit pero random

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "BALANCED MASKING TEST\n";
    std::cout << "=====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    // e ay SMALL RANDOM (1 to 1000) — mask pero hindi overwhem
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 1000 + 1);  // maliit na e
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };
    
    // Decrypt: ct mod Q, multiply by ψ^k
    // v·ψ^k = m·(-1)^k + e·ψ^(2k)
    // e ay maliit, kaya e·ψ^(2k) ay manageable
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ v_psi = (v * psi_k) % Q;
        
        // v_psi = m·(-1)^k + e·ψ^(2k)
        // Para sa k=42: (-1)^42 = 1, kaya v_psi = m + e·ψ^84
        // e·ψ^84 ay malaki pero mas maliit sa Q/2 kung e ay maliit
        
        // Check kung mas malapit sa 0 o sa 1
        NTL::ZZ dist_1 = (v_psi > 1) ? v_psi - 1 : 1 - v_psi;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (v_psi < Q/2) ? v_psi : Q - v_psi;
        
        return dist_1 < dist_0;
    };
    
    // Test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    
    std::cout << "Decrypt(ct0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(ct1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        // NAND
        NTL::ZZ a_mod = current % Q;
        NTL::ZZ prod = (a_mod * a_mod) % Q;
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ e = NTL::to_ZZ(rng() % 1000 + 1);
        NTL::ZZ r = NTL::RandomBnd(Q);
        current = result + e * psi_k + r * Q;
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
