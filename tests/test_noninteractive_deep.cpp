// NON-INTERACTIVE DEEP TEST
// 1000+ NAND sa outer nang walang decrypt-reencrypt

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "NON-INTERACTIVE DEEP TEST\n";
    std::cout << "=========================\n\n";
    
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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = inner + e·ψ^k + r·Q
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
        return inner + e * psi_k + r * Q;
    };
    
    // Decrypt
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    // NON-INTERACTIVE NAND: φ^k - (a mod Q)(b mod Q)·φ^(-k)
    // Walang decrypt! Walang re-encrypt!
    auto nand_noninteractive = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;  // Walang r·Q! Walang re-encrypt!
    };
    
    // Test
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain — NON-INTERACTIVE
    std::cout << "Deep chain (100 depths, non-interactive):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) {
            if (errors < 5) std::cout << "  Depth " << i << ": got " << dec << ", exp " << expected << " ✗\n";
            errors++;
        }
        current = nand_noninteractive(current, current);
    }
    std::cout << "  Errors: " << errors << "/101\n";
    
    return 0;
}
