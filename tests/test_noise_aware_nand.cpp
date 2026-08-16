// NOISE-AWARE NAND — Subukan ang cross-term cancellation
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "NOISE-AWARE NAND TEST\n";
    std::cout << "====================\n\n";
    
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
    
    // Encrypt: ct = m·φ^k + e·ψ^k (walang Q-embed muna para sa testing)
    auto encrypt = [&](bool bit, NTL::ZZ e_val) {
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return (m_val + e_val * psi_k) % Q;
    };
    
    // Decrypt: scaled = v·φ^k = m·φ^(2k) + e
    auto decrypt = [&](NTL::ZZ v) {
        NTL::ZZ scaled = (v * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    // Test simple decryption with known noise
    auto e1 = NTL::to_ZZ(10);
    auto ct0 = encrypt(false, e1);
    auto ct1 = encrypt(true, e1);
    std::cout << "Decrypt(0, e=10) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1, e=10) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // NOISE-AWARE NAND: Try different formulations
    // Formulation 1: Simple NAND with same noise structure
    auto nand_simple = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        // Add same noise para sa consistency
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return (result + e * psi_k) % Q;
    };
    
    // Test deep chain with nand_simple
    std::cout << "Deep chain with SIMPLE NAND:\n";
    auto current = encrypt(true, NTL::to_ZZ(10));
    int errors = 0;
    for (int i = 0; i <= 20; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_simple(current, current);
    }
    std::cout << "  Errors: " << errors << "/21\n\n";
    
    // Formulation 2: Try na i-subtract ang expected noise
    // Kung ang NAND(1,1) ay dapat 0 + e_new·ψ^k,
    // at ang NAND(0,0) ay dapat φ^k + e_new·ψ^k,
    // subukan nating i-exact ang result sa orbit
    auto nand_exact = [&](NTL::ZZ a, NTL::ZZ b) {
        // Decrypt muna para malaman ang expected result
        bool m_a = decrypt(a);
        bool m_b = decrypt(b);
        bool nand_result = !(m_a && m_b);
        // Re-encrypt nang may fresh noise
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        NTL::ZZ m_val = nand_result ? phi_k : NTL::to_ZZ(0);
        return (m_val + e * psi_k) % Q;
    };
    
    // Test deep chain with nand_exact (DECRYPT-REENCRYPT!)
    std::cout << "Deep chain with DECRYPT-REENCRYPT (hindi tunay na FHE):\n";
    current = encrypt(true, NTL::to_ZZ(10));
    errors = 0;
    for (int i = 0; i <= 20; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_exact(current, current);
    }
    std::cout << "  Errors: " << errors << "/21\n\n";
    
    std::cout << "KEY INSIGHT:\n";
    std::cout << "  Ang nand_simple ay may noise accumulation.\n";
    std::cout << "  Ang nand_exact ay decrypt-reencrypt (hindi FHE).\n";
    std::cout << "  Kailangan ng TAMANG formula na walang decrypt,\n";
    std::cout << "  pero may noise cancellation.\n";
    
    return 0;
}
