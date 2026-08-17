// HOMOMORPHIC DECRYPTION MULTI-TEST
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "HOMOMORPHIC DECRYPTION MULTI-TEST\n";
    std::cout << "==================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;

    std::mt19937_64 rng(42);

    // Multiple BKs with different noise
    auto make_BK = [&](NTL::ZZ e_bk) {
        NTL::ZZ r_bk = NTL::RandomBnd(Q);
        return phi_k + e_bk * psi_k + r_bk * Q;
    };

    auto recover = [&](NTL::ZZ ct, NTL::ZZ BK) {
        NTL::ZZ hom_dec = (ct * BK) % Q;
        NTL::ZZ scaled = (hom_dec * inv_phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };

    // Test sa iba't ibang noise levels
    std::cout << "BK noise | CT noise | Recovery rate\n";
    std::cout << "---------|----------|-------------\n";
    
    for (long e_bk : {1L, 5L, 10L, 50L, 100L}) {
        NTL::ZZ BK = make_BK(NTL::to_ZZ(e_bk));
        int success = 0, total = 100;
        
        for (int t = 0; t < total; t++) {
            bool bit = (t % 2 == 0);
            long e_ct = 1 + (t % 100);
            NTL::ZZ r_ct = NTL::RandomBnd(Q);
            NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
            NTL::ZZ ct = inner + NTL::to_ZZ(e_ct) * psi_k + r_ct * Q;
            
            bool recovered = recover(ct, BK);
            if (recovered == bit) success++;
        }
        
        std::cout << e_bk << " | 1-100 | " << success << "/" << total << "\n";
    }

    return 0;
}
