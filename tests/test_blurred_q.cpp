// BLURRED Q — I-blur ang Q para hindi ma-GCD
// ct = m·φ^k + r·Q + e (small noise)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "BLURRED Q APPROACH\n";
    std::cout << "==================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    
    std::mt19937_64 rng(42);
    
    // Blurred: ct = m·φ^k + r·Q + e (e ay maliit na noise)
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 1000 + 1);  // small noise
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + r * Q + e;
    };
    
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // v = m·φ^k + e
        // Kailangan i-check kung mas malapit sa 0 o φ^k
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    };
    
    // GCD attack test
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 10; i++) {
        cts.push_back(encrypt(false));  // lahat message 0
    }
    NTL::ZZ gcd_val = cts[1] - cts[0];
    if (gcd_val < 0) gcd_val = -gcd_val;
    for (int i = 2; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        gcd_val = NTL::GCD(gcd_val, diff);
    }
    
    std::cout << "GCD ng 10 blurred ciphertexts: " << gcd_val << "\n";
    std::cout << "Q = " << Q << "\n";
    std::cout << "Match: " << (gcd_val == Q ? "Q LEAKED! ✗" : "Q HIDDEN ✓") << "\n\n";
    
    // Decryption test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 1000 + 1);
        return result + r * Q + e;
    };
    
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
