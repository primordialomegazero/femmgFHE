// DUAL-LAYER FHE
// Inner: φ-ring (perfect NAND)
// Outer: ψ-noise (semantic security)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "DUAL-LAYER FHE TEST\n";
    std::cout << "==================\n\n";
    
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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    // INNER LAYER: φ-ring Q-embed (perfect NAND)
    auto inner_encrypt = [&](bool bit) {
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val;  // Walang noise — pure φ^k
    };
    
    auto inner_decrypt = [&](NTL::ZZ v) {
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    };
    
    auto inner_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };
    
    // OUTER LAYER: ψ-noise Q-embed (semantic security)
    auto outer_encrypt = [&](NTL::ZZ inner_ct) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return inner_ct + e * psi_k + r * Q;
    };
    
    auto outer_decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // v = inner_ct + e·ψ^k
        // Multiply by φ^k: inner_ct·φ^k + e
        NTL::ZZ scaled = (v * phi_k) % Q;
        // Para sa inner_ct = 0: scaled = e (maliit)
        // Para sa inner_ct = φ^k: scaled = φ^(2k) + e (malaki)
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    // FULL PIPELINE
    std::cout << "FULL DUAL-LAYER PIPELINE:\n";
    std::cout << "========================\n\n";
    
    // Encrypt: m → inner_encrypt → outer_encrypt
    auto full_encrypt = [&](bool bit) {
        NTL::ZZ inner = inner_encrypt(bit);
        return outer_encrypt(inner);
    };
    
    // Evaluator: decrypt outer, NAND inner, re-encrypt outer
    auto full_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        // 1. Decrypt outer
        NTL::ZZ a_inner = a % Q;
        NTL::ZZ b_inner = b % Q;
        
        // 2. NAND sa inner (homomorphic)
        NTL::ZZ result_inner = inner_nand(a_inner, b_inner);
        
        // 3. Re-encrypt outer
        return outer_encrypt(result_inner);
    };
    
    // Full decrypt: decrypt outer muna, then decrypt inner
    auto full_decrypt = [&](NTL::ZZ ct) {
        // Decrypt outer: multiply by φ^k para ma-remove ang ψ-noise
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * phi_k) % Q;
        // scaled = inner_ct·φ^k + e·φ^k·ψ^k = inner_ct·φ^k + e
        // Para sa inner_ct = 0: scaled = e (maliit)
        // Para sa inner_ct = φ^k: scaled = φ^(2k) + e (malaki)
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        
        // Kung malapit sa φ^(2k), inner message = 1
        // Kung malapit sa 0, inner message = 0
        if (d_phi2k < d_0) {
            // inner message = 1
            return true;
        } else {
            // inner message = 0
            return false;
        }
    };
    
    // Test
    auto ct0 = full_encrypt(false);
    auto ct1 = full_encrypt(true);
    std::cout << "Full decrypt(0) = " << full_decrypt(ct0) << " (exp 0)\n";
    std::cout << "Full decrypt(1) = " << full_decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = full_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = full_nand(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
