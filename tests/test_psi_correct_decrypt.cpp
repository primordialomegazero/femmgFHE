// CORRECTED DECRYPTION: ψ^(-k) = φ^k scaling
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "CORRECTED PSI-ANCHORED DECRYPTION\n";
    std::cout << "=================================\n\n";
    
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
    
    // VERIFY: ψ^(-k) = φ^k (k=42 even)
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);
    std::cout << "ψ^(-k) = " << inv_psi_k << "\n";
    std::cout << "φ^k = " << phi_k << "\n";
    std::cout << "Match: " << (inv_psi_k == phi_k ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::mt19937_64 rng(42);
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };
    
    // CORRECTED DECRYPT: v·ψ^(-k) = v·φ^k = m·φ^(2k) + e·φ^k·ψ^k
    // = m·φ^(2k) + e (since φ^k·ψ^k = 1)
    // m=0: e (maliit, < 100)
    // m=1: φ^(2k) (malaki)
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // Use ψ^(-k) = φ^k
        NTL::ZZ scaled = (v * phi_k) % Q;
        // scaled = m·φ^(2k) + e
        // Check kung malaki (m=1) o maliit (m=0)
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    // Test
    int correct = 0;
    for (int i = 0; i < 100; i++) {
        bool bit = i % 2 == 0;
        auto ct = encrypt(bit);
        bool dec = decrypt(ct);
        if (dec == bit) correct++;
    }
    std::cout << "Encryption/Decryption: " << correct << "/100\n\n";
    
    // NAND
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return result + e * psi_k + r * Q;
    };
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto current = encrypt(true);
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_op(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
