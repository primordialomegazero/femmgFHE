// ψ^k NOISE SA k=2048
// L(2048) = 1422 bits, ψ^2048 = 2048 bits
// Mas balanseng ratio

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ψ^k NOISE SA k=2048\n";
    std::cout << "==================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    long k = 2048;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::cout << "1. SIZES:\n";
    std::cout << "   L(k) bits: " << NTL::NumBits(L_k) << "\n";
    std::cout << "   ψ^k bits: " << NTL::NumBits(psi_k) << "\n";
    std::cout << "   Q bits: " << NTL::NumBits(Q) << "\n\n";

    std::mt19937_64 rng(42);

    auto encrypt = [&](bool bit, NTL::ZZ e_val) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + e_val * psi_k + r * Q;
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

    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_L = (v > L_k) ? v - L_k : L_k - v;
        if (d_L > Q/2) d_L = Q - d_L;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_L < d_0;
    };

    // Test with various noise levels
    std::cout << "2. TEST SA IBA'T IBANG NOISE LEVELS:\n";
    std::cout << "   e | Decrypt(1) | NAND(0,0) | NAND(1,1)\n";
    std::cout << "   --|------------|-----------|----------\n";

    for (long e : {1L, 2L, 5L, 10L, 50L, 100L}) {
        auto ct0 = encrypt(false, NTL::to_ZZ(e));
        auto ct1 = encrypt(true, NTL::to_ZZ(e));
        
        bool dec1 = decrypt(ct1);
        bool nand00 = decrypt(nand(ct0, ct0));
        bool nand11 = decrypt(nand(ct1, ct1));
        
        std::cout << "   " << e << " | " << dec1 << " | " << nand00 << " | " << nand11 << "\n";
    }

    std::cout << "\n3. DEEP CHAIN TEST (100 depths):\n";
    for (long e : {1L, 5L, 10L}) {
        auto current = encrypt(true, NTL::to_ZZ(e));
        int errors = 0;
        for (int i = 0; i <= 100; i++) {
            bool dec = decrypt(current);
            bool expected = (i % 2 == 0) ? 1 : 0;
            if (dec != expected) errors++;
            current = nand(current, current);
        }
        std::cout << "   e=" << e << ": " << errors << "/101 errors\n";
    }

    return 0;
}
