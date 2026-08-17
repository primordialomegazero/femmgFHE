#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "DUAL-LAYER DEBUG\n\n";
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

    auto inner_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto outer_decrypt_inner = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return (d_phi2k < d_0) ? phi_k : NTL::to_ZZ(0);
    };

    auto outer_encrypt = [&](NTL::ZZ inner) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return inner + e * psi_k + r * Q;
    };

    // Start with inner = φ^k (message 1)
    NTL::ZZ inner = phi_k;
    std::cout << "Depth | Inner (dec) | Expected\n";
    std::cout << "------|-------------|---------\n";

    for (int i = 0; i <= 10; i++) {
        NTL::ZZ outer = outer_encrypt(inner);
        NTL::ZZ recovered = outer_decrypt_inner(outer);
        std::cout << i << " | " << (recovered == phi_k ? 1 : 0) << " | " << (i % 2 == 0) << "\n";

        // NAND(inner, inner) → NOT-like
        inner = inner_nand(inner, inner);
    }

    return 0;
}
