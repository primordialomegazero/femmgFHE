// MULTIPLICATIVE φ-NOISE MODEL
// ct = (m + e)·φ^k + r·Q — noise ay multiplicative sa message

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "MULTIPLICATIVE φ-NOISE TEST\n";
    std::cout << "===========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = (m + e)·φ^k + r·Q
    // e ay maliit (0-10)
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 10);  // e ∈ [0, 9]
        NTL::ZZ m_val = bit ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        return (m_val + e) * phi_k + r * Q;
    };
    
    // Decrypt: v·φ^(-k) = m + e
    // m=0: e ∈ [0, 9]
    // m=1: 1+e ∈ [1, 10]
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * inv_phi_k) % Q;
        // scaled = m + e
        // Para sa m=1: 1+e ∈ [1, 10] — malapit sa 1..10
        // Para sa m=0: e ∈ [0, 9] — malapit sa 0..9
        // OVERLAP sa [1, 9]!
        // Kailangan ng better decision
        NTL::ZZ dist_1 = (scaled > 1) ? scaled - 1 : 1 - scaled;
        NTL::ZZ dist_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return dist_1 < dist_0;
    };
    
    std::cout << "1. NOISE RANGE CHECK:\n";
    std::cout << "   m=0: scaled ∈ [0, 9]\n";
    std::cout << "   m=1: scaled ∈ [1, 10]\n";
    std::cout << "   OVERLAP sa [1, 9] — hindi ma-distinguish!\n\n";
    
    // Test
    int correct = 0;
    for (int i = 0; i < 100; i++) {
        bool bit = i % 2 == 0;
        auto ct = encrypt(bit);
        bool dec = decrypt(ct);
        if (dec == bit) correct++;
    }
    std::cout << "2. ENCRYPTION/DECRYPTION: " << correct << "/100\n\n";
    
    // Try smaller noise: e ∈ [0, 1]
    auto encrypt_small = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 2);  // e ∈ {0, 1}
        NTL::ZZ m_val = bit ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        return (m_val + e) * phi_k + r * Q;
    };
    
    correct = 0;
    for (int i = 0; i < 100; i++) {
        bool bit = i % 2 == 0;
        auto ct = encrypt_small(bit);
        bool dec = decrypt(ct);
        if (dec == bit) correct++;
    }
    std::cout << "3. SMALL NOISE (e ∈ {{0,1}}): " << correct << "/100\n\n";
    
    // NAND with multiplicative noise
    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 2);
        return result + e * phi_k + r * Q;
    };
    
    std::cout << "4. DEEP CHAIN (small noise):\n";
    auto current = encrypt_small(true);
    int errors = 0;
    for (int i = 0; i <= 50; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/51\n\n";
    
    std::cout << "5. KEY INSIGHT:\n";
    std::cout << "   Ang multiplicative φ-noise ay may OVERLAP issue\n";
    std::cout << "   sa decryption (m=0 at m=1 ay magkalapit).\n";
    std::cout << "   → Kailangan ng mas malaking separation\n";
    std::cout << "   → O ibang encoding na may mas malinaw na boundary\n";
    
    return 0;
}
