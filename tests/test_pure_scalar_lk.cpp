// PURE SCALAR L(k) FHE
// Walang polynomial ring — puro scalar operations

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "PURE SCALAR L(k) FHE\n";
    std::cout << "===================\n\n";

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

    std::mt19937_64 rng(42);

    std::cout << "1. ANG SETUP:\n";
    std::cout << "   Public: L(k) = " << L_k << "\n";
    std::cout << "   Public: inv_L(k) = " << inv_L_k << "\n\n";

    // Scalar RLWE-style encryption
    // ct = m + e·ψ^k + r·Q
    // Sa halip na polynomial noise, scalar noise lamang
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 10);  // maliit na scalar noise
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };

    // Non-interactive NAND sa L(k) space
    // NAND(a,b) = L(k) - (a mod Q)·(b mod Q)·inv_L(k)
    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Decrypt: v = ct mod Q
    // v = m·L(k) + e·ψ^k
    // Kung m=0: v = e·ψ^k (malaki — ψ^k ay malaki)
    // Kung m=1: v = L(k) + e·ψ^k (malaki pa rin!)
    // PROBLEM: Ang ψ^k ay malaki kaya mahirap i-distinguish
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // Kailangan i-project sa L(k) direction
        // v·inv_L(k) = m + e·ψ^k·inv_L(k)
        NTL::ZZ scaled = (v * inv_L_k) % Q;
        // Kung m=0: e·ψ^k·inv_L(k) — malaki
        // Kung m=1: 1 + e·ψ^k·inv_L(k) — malaki pa rin
        // HINDI ito gumagana!
        NTL::ZZ d_1 = (scaled > 1) ? scaled - 1 : 1 - scaled;
        if (d_1 > Q/2) d_1 = Q - d_1;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_1 < d_0;
    };

    std::cout << "2. ENCRYPT/DECRYPT TEST:\n";
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "   Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";

    std::cout << "3. NAND TEST:\n";
    auto n00 = nand(ct0, ct0);
    auto n01 = nand(ct0, ct1);
    auto n11 = nand(ct1, ct1);
    std::cout << "   NAND(0,0) = " << decrypt(n00) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt(n01) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt(n11) << " (exp 0)\n\n";

    std::cout << "4. DEEP CHAIN:\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/101\n";

    return 0;
}
