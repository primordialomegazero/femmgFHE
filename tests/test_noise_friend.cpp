// NOISE AS FRIEND — ψ^k bilang public projection key
// Public: Q, ψ^k
// Secret: φ^k

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "NOISE AS FRIEND TEST\n";
    std::cout << "====================\n\n";

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

    std::mt19937_64 rng(42);

    std::cout << "1. KEY SETUP:\n";
    std::cout << "   Public: ψ^k = " << psi_k << "\n";
    std::cout << "   Secret: φ^k = " << phi_k << "\n\n";

    // Encrypt: ct = m·φ^k + e·ψ^k + r·Q
    auto encrypt = [&](bool bit, long e_val) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(e_val);
        NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
        return inner + e * psi_k + r * Q;
    };

    // Homomorphic projection gamit ang ψ^k (PUBLIC)
    auto hom_project = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        return (v * psi_k) % Q;
    };

    // Decrypt: i-check ang projection
    // ct·ψ^k = m·φ^k·ψ^k + e·ψ^(2k) = m + e·ψ^(2k)
    // m=1: 1 + e·ψ^(2k)
    // m=0: e·ψ^(2k)
    auto decrypt_proj = [&](NTL::ZZ proj) {
        // Kailangan i-distinguish ang 1 + noise mula sa 0 + noise
        // Kung ang noise ay maliit, ang 1 ay recoverable
        NTL::ZZ dist_1 = (proj > 1) ? proj - 1 : 1 - proj;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (proj < Q/2) ? proj : Q - proj;
        return dist_1 < dist_0;
    };

    std::cout << "2. SINGLE RECOVERY TEST:\n";
    for (long e : {1L, 5L, 10L}) {
        auto ct0 = encrypt(false, e);
        auto ct1 = encrypt(true, e);
        auto proj0 = hom_project(ct0);
        auto proj1 = hom_project(ct1);
        bool dec0 = decrypt_proj(proj0);
        bool dec1 = decrypt_proj(proj1);
        
        std::cout << "   e=" << e << ": m=0 → " << (dec0 ? 1 : 0) 
                  << " (exp 0), m=1 → " << (dec1 ? 1 : 0) << " (exp 1)\n";
    }

    std::cout << "\n3. MULTI-TEST (100 messages):\n";
    for (long e_max : {1L, 5L, 10L}) {
        int success = 0, total = 200;
        for (int t = 0; t < total; t++) {
            bool bit = (t % 2 == 0);
            long e_ct = 1 + (t % e_max);
            auto ct = encrypt(bit, e_ct);
            auto proj = hom_project(ct);
            bool dec = decrypt_proj(proj);
            if (dec == bit) success++;
        }
        std::cout << "   e_max=" << e_max << ": " << success << "/" << total << "\n";
    }

    return 0;
}
