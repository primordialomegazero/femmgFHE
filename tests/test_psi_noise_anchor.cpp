// PSI NOISE ANCHOR
// Noise = e·ψ^k, kasi ψ^k·φ^(-k) = (-1)^k (constant!)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "PSI NOISE ANCHOR TEST\n";
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
    
    // VERIFY: ψ^k · φ^(-k) = (-1)^k = 1 (k=42 even)
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ check = (psi_k * inv_phi_k) % Q;
    std::cout << "ψ^k · φ^(-k) mod Q = " << check << " (should be 1)\n\n";
    
    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    // Decrypt: ct mod Q = m·φ^k + e·ψ^k
    // Multiply by φ^(-k): m + e·ψ^k·φ^(-k) = m + e·(-1)^k = m + e
    // Since e·(-1)^k = e (k=42 even), ang decrypt ay m + e
    // Kung e < Q/2, ang m ay recoverable
    
    std::mt19937_64 rng(42);
    
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);  // small e
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };
    
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // v = m·φ^k + e·ψ^k
        // Multiply by φ^(-k): m + e·(-1)^k = m + e (k=42 even)
        NTL::ZZ v_scaled = (v * inv_phi_k) % Q;
        // v_scaled = m + e (maliit ang e)
        NTL::ZZ dist_1 = (v_scaled > 1) ? v_scaled - 1 : 1 - v_scaled;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (v_scaled < Q/2) ? v_scaled : Q - v_scaled;
        return dist_1 < dist_0;
    };
    
    // Test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // GCD attack
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 10; i++) cts.push_back(encrypt(false));
    NTL::ZZ gcd_val = cts[1] - cts[0];
    if (gcd_val < 0) gcd_val = -gcd_val;
    for (int i = 2; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        gcd_val = NTL::GCD(gcd_val, diff);
    }
    std::cout << "GCD attack: " << (gcd_val == Q ? "Q LEAKED ✗" : "Q HIDDEN ✓") << "\n\n";
    
    // Deep chain
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
