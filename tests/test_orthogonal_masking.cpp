// ORTHOGONAL MASKING — ψ-direction noise, φ-direction message
// ct = m·φ^k + e·ψ^k, decrypt via ψ projection

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ORTHOGONAL MASKING TEST\n";
    std::cout << "=======================\n\n";
    
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
    
    // CHECK: φ^k · ψ^k = 1 (k=42 even)
    NTL::ZZ check = (phi_k * psi_k) % Q;
    std::cout << "φ^k · ψ^k mod Q = " << check << " (should be 1)\n\n";
    
    // KEY: Project sa φ^k para makuha ang m
    // ct · φ^k = m·φ^(2k) + e·φ^k·ψ^k = m·φ^(2k) + e
    // Hmm, hindi ito naghihiwalay.
    
    // TAMANG PROJECTION: ct · ψ^k
    // ct · ψ^k = m·φ^k·ψ^k + e·ψ^(2k) = m + e·ψ^(2k)
    // Kung e ay MALIIT (0 o 1), ang m ay recoverable
    
    // SUBUKAN: e = 0 o 1 lang
    std::mt19937_64 rng(42);
    
    auto encrypt = [&](bool bit) {
        NTL::ZZ e = NTL::to_ZZ(rng() % 2);  // e ∈ {0, 1}
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k;
    };
    
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v_psi = (ct * psi_k) % Q;
        // v_psi = m + e·ψ^(2k)
        // Para sa e=0: v_psi = m ∈ {0, 1}
        // Para sa e=1: v_psi = m + ψ^(2k) — malaking value
        NTL::ZZ dist_1 = (v_psi > 1) ? v_psi - 1 : 1 - v_psi;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (v_psi < Q/2) ? v_psi : Q - v_psi;
        return dist_1 < dist_0;
    };
    
    // Test
    int correct = 0, total = 100;
    for (int i = 0; i < total; i++) {
        bool bit = i % 2 == 0;
        auto ct = encrypt(bit);
        bool dec = decrypt(ct);
        if (dec == bit) correct++;
    }
    std::cout << "Encryption/Decryption: " << correct << "/" << total << "\n\n";
    
    // NAND with masking
    auto nand_masked = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ e = NTL::to_ZZ(rng() % 2);
        return result + e * psi_k;
    };
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto current = encrypt(true);
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_masked(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
