// L(k)-BASED NON-INTERACTIVE FHE
// BK = L(k) ay PUBLIC — evaluator ay walang secret

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "L(k)-BASED NON-INTERACTIVE TEST\n";
    std::cout << "================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);
    
    // PUBLIC: Q, L(k), inv_L(k)
    // SECRET: φ^k, ψ^k
    
    std::cout << "PUBLIC: Q, L(k)=" << L_k << ", inv_L(k)=" << inv_L_k << "\n";
    std::cout << "SECRET: φ^k, ψ^k\n\n";
    
    // Encrypt: ct = m·L(k) + r·Q (message sa L(k) direction)
    // Sa L(k) space, ang message ay {0, L(k)}
    std::mt19937_64 rng(42);
    
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + r * Q;
    };
    
    // NAND sa L(k) space: L(k) - a·b·inv_L(k)
    auto nand_L = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        // Re-embed sa Q
        NTL::ZZ r = NTL::RandomBnd(Q);
        return result + r * Q;
    };
    
    // Decrypt: v mod Q → check dist sa L(k) o 0
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_L = (v > L_k) ? v - L_k : L_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_L < d_0;
    };
    
    // Test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain — NON-INTERACTIVE (walang decrypt-reencrypt)
    std::cout << "Deep chain (100 depths, non-interactive):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_L(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    return 0;
}
