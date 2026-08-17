// ADDITIVE ELGAMAL NAND — Homomorphic NAND via division sa exponent
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ADDITIVE ELGAMAL NAND TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    // Simplified: use multiplicative group Z_Q*
    NTL::ZZ g = NTL::to_ZZ(2);  // generator
    NTL::ZZ k = NTL::to_ZZ(42); // secret key
    NTL::ZZ h = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) h = (h * g) % Q;
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = (g^r, g^m · h^r)
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::to_ZZ(rng() % 100 + 1);
        NTL::ZZ c1 = NTL::to_ZZ(1);
        NTL::ZZ c2 = NTL::to_ZZ(1);
        for (int i = 0; i < NTL::conv<long>(r); i++) {
            c1 = (c1 * g) % Q;
            c2 = (c2 * h) % Q;
        }
        NTL::ZZ g_m = bit ? g : NTL::to_ZZ(1);
        c2 = (g_m * c2) % Q;
        return std::make_pair(c1, c2);
    };
    
    // Decrypt: c2 / c1^k = g^m → check kung 1 (m=0) o g (m=1)
    auto decrypt = [&](std::pair<NTL::ZZ, NTL::ZZ> ct) {
        NTL::ZZ c1_k = NTL::to_ZZ(1);
        for (int i = 0; i < NTL::conv<long>(k); i++) c1_k = (c1_k * ct.first) % Q;
        NTL::ZZ c1_k_inv = NTL::InvMod(c1_k, Q);
        NTL::ZZ g_m = (ct.second * c1_k_inv) % Q;
        
        // g_m = 1 → m=0, g_m = g → m=1
        NTL::ZZ dist_1 = (g_m > 1) ? g_m - 1 : 1 - g_m;
        NTL::ZZ dist_g = (g_m > g) ? g_m - g : g - g_m;
        return dist_g < dist_1;
    };
    
    // HOMOMORPHIC NAND: (g^r1, g^m1·h^r1) NAND (g^r2, g^m2·h^r2)
    // = (g^(r1+r2), g/(g^m1·h^r1 · g^m2·h^r2))
    // WAIT — kailangan ng h^(r1+r2) para sa decryption
    // NAND output: c1 = c1_a · c1_b = g^(r_a+r_b)
    // c2 = g · h^(r_a+r_b) / (g^m_a·h^r_a · g^m_b·h^r_b)
    // = g · h^(r_a+r_b) / (g^(m_a+m_b) · h^(r_a+r_b))
    // = g^(1-m_a-m_b)
    // Para sa NAND: 1 - m_a·m_b ≠ 1 - m_a - m_b
    
    // PROBLEM: Hindi ito NAND — ito ay NOR-like
    // g^(1-m_a-m_b) ay naka-depend sa m_a+m_b, hindi m_a·m_b
    
    std::cout << "PROBLEM: g^(1-m_a-m_b) ≠ g^(1-m_a·m_b)\n";
    std::cout << "Ang homomorphic multiplication ay nagbibigay ng\n";
    std::cout << "ADDITION sa exponent, hindi MULTIPLICATION.\n";
    std::cout << "Kailangan ng ibang encoding para sa NAND.\n\n";
    
    std::cout << "ALTERNATIVE: g^(m_a·m_b) via repeated addition?\n";
    std::cout << "Hindi practical para sa binary values.\n\n";
    
    std::cout << "=== ADDITIVE ELGAMAL NAND: HINDI STRAIGHTFORWARD ===\n";
    
    return 0;
}
