// INTEGER L(k) UNIVERSAL
// L(k) = F(k+1) + F(k-1) — integer, independent sa Q

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "INTEGER L(k) UNIVERSAL TEST\n";
    std::cout << "===========================\n\n";

    // Compute integer L(k) — walang modulo
    long k = 42;
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i <= k+1; i++) {
        fib[i] = fib[i-1] + fib[i-2];  // INTEGER addition, walang modulo!
    }
    NTL::ZZ L_integer = fib[k+1] + fib[k-1];

    std::cout << "1. INTEGER L(k):\n";
    std::cout << "   L(42) = F(43) + F(41)\n";
    std::cout << "   = " << fib[43] << " + " << fib[41] << "\n";
    std::cout << "   = " << L_integer << "\n\n";

    // Test sa Q_large at Q_small
    NTL::ZZ Q_large = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ Q_small = NTL::to_ZZ("340282366920938463463374607431768211507");

    std::cout << "2. VERIFY UNIVERSALITY:\n";
    std::cout << "   L(k) mod Q_large = " << (L_integer % Q_large) << "\n";
    std::cout << "   L(k) mod Q_small = " << (L_integer % Q_small) << "\n";
    std::cout << "   Pareho (dahil L(k) < Q_small): " 
              << ((L_integer % Q_large) == (L_integer % Q_small) ? "YES ✓" : "NO") << "\n\n";

    // ============ FULL FHE SA Q_SMALL ============
    NTL::ZZ Q = Q_small;
    NTL::ZZ L_k = L_integer % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::mt19937_64 rng(42);

    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + r * Q;
    };

    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_L = (v > L_k) ? v - L_k : L_k - v;
        if (d_L > Q/2) d_L = Q - d_L;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_L < d_0;
    };

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

    std::cout << "3. FHE SA Q_SMALL (129-bit):\n";
    std::cout << "   L(k) = " << L_k << "\n";
    std::cout << "   inv_L(k) = " << inv_L_k << "\n\n";

    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);

    std::cout << "   Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n";
    std::cout << "   NAND(0,0) = " << decrypt(nand(ct0, ct0)) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt(nand(ct0, ct1)) << " (exp 1)\n";
    std::cout << "   NAND(1,0) = " << decrypt(nand(ct1, ct0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt(nand(ct1, ct1)) << " (exp 0)\n\n";

    std::cout << "4. DEEP CHAIN (1000 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 1000; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/1001\n\n";

    // ============ SA Q_LARGE ============
    NTL::ZZ Q_l = Q_large;
    NTL::ZZ L_l = L_integer % Q_l;
    NTL::ZZ inv_L_l = NTL::InvMod(L_l, Q_l);

    auto encrypt_l = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q_l);
        NTL::ZZ m_val = bit ? L_l : NTL::to_ZZ(0);
        return m_val + r * Q_l;
    };

    auto decrypt_l = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q_l;
        if (v < 0) v += Q_l;
        NTL::ZZ d_L = (v > L_l) ? v - L_l : L_l - v;
        if (d_L > Q_l/2) d_L = Q_l - d_L;
        NTL::ZZ d_0 = (v < Q_l/2) ? v : Q_l - v;
        return d_L < d_0;
    };

    auto nand_l = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q_l;
        if (a_mod < 0) a_mod += Q_l;
        NTL::ZZ b_mod = b % Q_l;
        if (b_mod < 0) b_mod += Q_l;
        NTL::ZZ prod = (a_mod * b_mod) % Q_l;
        NTL::ZZ scaled = (prod * inv_L_l) % Q_l;
        NTL::ZZ result = (L_l - scaled) % Q_l;
        if (result < 0) result += Q_l;
        return result;
    };

    auto ct0_l = encrypt_l(false);
    auto ct1_l = encrypt_l(true);

    std::cout << "5. FHE SA Q_LARGE (257-bit):\n";
    std::cout << "   Decrypt(0) = " << decrypt_l(ct0_l) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt_l(ct1_l) << " (exp 1)\n";
    std::cout << "   NAND(0,0) = " << decrypt_l(nand_l(ct0_l, ct0_l)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt_l(nand_l(ct1_l, ct1_l)) << " (exp 0)\n\n";

    std::cout << "6. DEEP CHAIN SA Q_LARGE (1000 depths):\n";
    current = ct1_l;
    errors = 0;
    for (int i = 0; i <= 1000; i++) {
        bool dec = decrypt_l(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_l(current, current);
    }
    std::cout << "   Errors: " << errors << "/1001\n";

    return 0;
}
