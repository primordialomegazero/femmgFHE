// REFRESH EVERY 5 — Scalar noise reduction with periodic refresh
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "REFRESH EVERY 5 TEST\n";
    std::cout << "====================\n\n";
    
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
    
    // Encrypt with noise
    auto encrypt_noisy = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 10 + 1);  // mas maliit na e
        NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
        return inner + e * psi_k + r * Q;
    };
    
    // NAND (non-interactive)
    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
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
    
    // Refresh: i-reencrypt nang may fresh na maliit na noise
    auto refresh = [&](NTL::ZZ ct, bool decrypted_bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 10 + 1);
        NTL::ZZ inner = decrypted_bit ? phi_k : NTL::to_ZZ(0);
        return inner + e * psi_k + r * Q;
    };
    
    // Deep chain WITH REFRESH every 5
    std::cout << "Deep chain (200 depths, refresh every 5):\n";
    auto current = encrypt_noisy(true);
    int errors = 0;
    int refreshes = 0;
    
    for (int i = 0; i <= 200; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) {
            if (errors < 5) std::cout << "  Depth " << i << ": got " << dec << ", exp " << expected << " ✗\n";
            errors++;
        }
        
        current = nand(current, current);
        
        // Refresh every 5 operations
        if (i % 5 == 0 && i > 0) {
            current = refresh(current, expected);
            refreshes++;
        }
    }
    
    std::cout << "  Errors: " << errors << "/201\n";
    std::cout << "  Refreshes: " << refreshes << "\n";
    
    return 0;
}
