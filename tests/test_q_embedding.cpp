// RANDOM Q-EMBEDDING TEST
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RANDOM Q-EMBEDDING TEST\n";
    std::cout << "=======================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ inv_phi = NTL::InvMod(phi, Q);
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = m·φ + r·Q
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::to_ZZ(rng() % 1000000 + 1);
        NTL::ZZ ct = (bit ? phi : NTL::to_ZZ(0)) + r * Q;
        return ct;
    };
    
    // Decrypt: ct mod Q = m·φ
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ d_phi = (v > phi) ? v - phi : phi - v;
        NTL::ZZ d_psi = (v > Q - phi) ? v - (Q - phi) : (Q - phi) - v;
        NTL::ZZ dist_phi_orbit = (d_phi < d_psi) ? d_phi : d_psi;
        return dist_phi_orbit < dist_0;
    };
    
    // NAND: φ - (a·b mod Q)·φ⁻¹
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_phi) % Q;
        NTL::ZZ result = (phi - scaled) % Q;
        if (result < 0) result += Q;
        // I-embed ulit sa Q para sa susunod na NAND
        NTL::ZZ r = NTL::to_ZZ(rng() % 1000000 + 1);
        return result + r * Q;
    };
    
    // Test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    
    std::cout << "Encrypt(0) = " << ct0 << "\n";
    std::cout << "Encrypt(1) = " << ct1 << "\n\n";
    
    std::cout << "Decrypt(ct0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(ct1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain
    auto current = ct1;
    std::cout << "Deep chain (100 depths):\n";
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
