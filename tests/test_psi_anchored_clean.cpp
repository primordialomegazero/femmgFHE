// PSI-ANCHORED Q-EMBED FHE — Clean Implementation
// ct = m·φ^k + e·ψ^k + r·Q

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "PSI-ANCHORED Q-EMBED FHE\n";
    std::cout << "========================\n\n";
    
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
    
    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };
    
    // Decrypt: v = ct mod Q, scaled = v·φ^(-k)
    // scaled = m + e·ψ^k·φ^(-k)
    // Since ψ^k·φ^(-k) = (-1)^k = 1 (k=42 even)
    // scaled = m + e (maliit ang e)
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * inv_phi_k) % Q;
        
        // scaled = m + e (e ∈ [1, 100])
        // m=0: scaled ∈ [1, 100]
        // m=1: scaled ∈ [101, 200] — malapit sa 1 pa rin!
        // PROBLEM: Ang e ay mas malaki sa 1, kaya ang m=0 at m=1 ay
        // parehong malapit sa 0... kailangan ng better decision
        // FIX: Use ψ^(-k) instead of φ^(-k)
        NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);
        NTL::ZZ scaled_psi = (v * inv_psi_k) % Q;
        // scaled_psi = m·φ^k·ψ^(-k) + e
        // = m·φ^k·ψ^(-k) + e
        // = m·(φ/ψ)^k + e
        // = m·φ^(2k) + e (since ψ^(-k) = φ^k / (φψ)^k = φ^k)
        // Actually: ψ^(-k) = φ^k (since φψ = -1, ψ^(-1) = -φ)
        // So scaled_psi = m·φ^(2k) + e
        
        NTL::ZZ dist_phi2k = (scaled_psi > phi_k*phi_k) ? scaled_psi - phi_k*phi_k : phi_k*phi_k - scaled_psi;
        NTL::ZZ dist_0 = (scaled_psi < Q/2) ? scaled_psi : Q - scaled_psi;
        return dist_phi2k < dist_0;
    };
    
    // Test encryption/decryption
    int correct = 0;
    for (int i = 0; i < 10; i++) {
        bool bit = i % 2 == 0;
        auto ct = encrypt(bit);
        bool dec = decrypt(ct);
        if (dec == bit) correct++;
    }
    std::cout << "Encryption/Decryption: " << correct << "/10\n\n";
    
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
