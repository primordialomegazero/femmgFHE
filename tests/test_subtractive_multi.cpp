// SUBTRACTIVE BK MULTI-TEST
// BK_φ - BK_ψ = φ^k - ψ^k = √5·F(k)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "SUBTRACTIVE BK MULTI-TEST\n";
    std::cout << "=========================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ inv_phi_2k = NTL::InvMod(phi_2k, Q);

    std::mt19937_64 rng(42);

    // BK_subtractive = φ^k - ψ^k (plain — baseline)
    NTL::ZZ BK_sub = (phi_k - psi_k + Q) % Q;

    std::cout << "1. BK_subtractive = φ^k - ψ^k\n";
    std::cout << "   = " << BK_sub << "\n\n";

    // Recovery function
    auto recover = [&](NTL::ZZ ct) {
        NTL::ZZ result = (ct * BK_sub) % Q;
        NTL::ZZ scaled = (result * inv_phi_2k) % Q;
        
        // scaled = 1 + small_noise (m=1) o small_noise (m=0)
        NTL::ZZ dist_1 = (scaled > 1) ? scaled - 1 : 1 - scaled;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return dist_1 < dist_0;
    };

    // Test sa iba't ibang noise levels
    std::cout << "2. RECOVERY TEST (1000 messages):\n";
    std::cout << "   Noise | Success rate\n";
    std::cout << "   ------|-------------\n";

    for (long e_max : {1L, 5L, 10L, 50L, 100L}) {
        int success = 0;
        int total = 200;
        
        for (int t = 0; t < total; t++) {
            bool bit = (t % 2 == 0);
            long e_ct = 1 + (t % e_max);
            NTL::ZZ r_ct = NTL::RandomBnd(Q);
            NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
            NTL::ZZ ct = inner + NTL::to_ZZ(e_ct) * psi_k + r_ct * Q;
            
            bool recovered = recover(ct);
            if (recovered == bit) success++;
        }
        
        std::cout << "   " << e_max << " | " << success << "/" << total << "\n";
    }

    return 0;
}
