// FULLY RANDOM EMBEDDING
// ct = (m·φ^k + e·ψ^k) + r·Q — mask ang φ^k mismo

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "FULLY RANDOM EMBEDDING TEST\n";
    std::cout << "===========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    // φ^k at ψ^k
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    // e ay random — mask ang φ^k value
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::RandomBnd(Q/2);  // malaking random e
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };
    
    // Decrypt: ct mod Q, project sa φ-direction
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // v = m·φ^k + e·ψ^k
        // Multiply by ψ^k: v·ψ^k = m·φ^k·ψ^k + e·ψ^(2k)
        // = m·(-1)^k + e·ψ^(2k) = m + e·ψ^(2k) (k=42 even)
        NTL::ZZ v_psi = (v * psi_k) % Q;
        // Check dist sa 1 (m=1) o 0 (m=0)
        NTL::ZZ dist_1 = (v_psi > 1) ? v_psi - 1 : 1 - v_psi;
        NTL::ZZ dist_0 = (v_psi < Q/2) ? v_psi : Q - v_psi;
        return dist_1 < dist_0;
    };
    
    // NAND: φ^k - (a·b mod Q)·(φ^k)⁻¹
    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ e = NTL::RandomBnd(Q/2);
        NTL::ZZ r = NTL::RandomBnd(Q);
        return result + e * psi_k + r * Q;
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
        current = nand(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
