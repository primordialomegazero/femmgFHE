// TRUE NOISE MEASUREMENT — SCALAR φ-FHE
// Hindi value level. Direktang sinusukat ang noise = |decoded - expected|
// Kung may natural noise cancellation ba talaga sa φ-structure.

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE NOISE — SCALAR φ-FHE\n";
    std::cout << "  noise = |decoded - expected|\n";
    std::cout << "========================================\n\n";

    // 257-bit prime Q ≡ 1 (mod 5)
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    // φ^k at ψ^k
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::cout << "PARAMETERS:\n";
    std::cout << "  φ^k = " << phi_k << "\n";
    std::cout << "  ψ^k = " << psi_k << "\n";
    std::cout << "  φ^k · ψ^k = " << (phi_k * psi_k) % Q << " (dapat 1)\n\n";

    // ============================================
    // ENCRYPTION na may controllable noise
    // ct = m·φ^k + e·ψ^k
    // decoded = ct · φ^(-k) = m + e·(ψ^k·φ^(-k))
    // = m + e·(-1)^k = m + e (k=42 even → +e)
    // ============================================

    auto encrypt = [&](int m, NTL::ZZ e) {
        // ct = m·φ^k + e·ψ^k
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    auto decode = [&](NTL::ZZ ct) {
        // decoded = ct · φ^(-k) mod Q
        NTL::ZZ decoded = (ct * inv_phi_k) % Q;
        if (decoded < 0) decoded += Q;
        // I-normalize sa center (para makita ang negative noise)
        if (decoded > Q/2) decoded -= Q;
        return decoded;
    };

    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        // NAND(a,b) = φ^k - (a·b)·φ^(-k)
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // ============================================
    // TEST 1: BASELINE NOISE — walang NAND
    // ============================================
    std::cout << "TEST 1: ENCRYPTION NOISE (baseline)\n";
    std::cout << "-----------------------------------\n";
    std::cout << "  m=0, e=0 → decoded = " << decode(encrypt(0, NTL::to_ZZ(0))) << "\n";
    std::cout << "  m=1, e=0 → decoded = " << decode(encrypt(1, NTL::to_ZZ(0))) << "\n";
    std::cout << "  m=0, e=5 → decoded = " << decode(encrypt(0, NTL::to_ZZ(5))) << " (noise=5)\n";
    std::cout << "  m=1, e=5 → decoded = " << decode(encrypt(1, NTL::to_ZZ(5))) << " (noise=5)\n\n";

    // ============================================
    // TEST 2: NAND NOISE GROWTH — e=0 (perfect)
    // ============================================
    std::cout << "TEST 2: NAND NOISE — e=0 (perfect start)\n";
    std::cout << "-----------------------------------\n";

    auto current = encrypt(1, NTL::to_ZZ(0));  // m=1, noise=0
    std::vector<long long> noise_levels;

    std::cout << "  Depth | Expected | Decoded | Noise\n";
    std::cout << "  ------|----------|---------|------\n";

    for (int i = 0; i <= 20; i++) {
        long long decoded = NTL::to_long(decode(current));
        long long expected = (i % 2 == 0) ? 1 : 0;
        long long noise = decoded - expected;

        std::cout << "  " << i << "     | " << expected << "        | "
                  << decoded << "       | " << noise << "\n";

        noise_levels.push_back(noise);
        current = nand_op(current, current);
    }

    // ============================================
    // TEST 3: NAND NOISE GROWTH — e=1 (may initial noise)
    // ============================================
    std::cout << "\nTEST 3: NAND NOISE — e=1 (may initial noise)\n";
    std::cout << "-----------------------------------\n";

    current = encrypt(1, NTL::to_ZZ(1));  // m=1, noise=1
    std::vector<long long> noise_growth;

    std::cout << "  Depth | Expected | Decoded | Noise\n";
    std::cout << "  ------|----------|---------|------\n";

    for (int i = 0; i <= 20; i++) {
        long long decoded = NTL::to_long(decode(current));
        long long expected = (i % 2 == 0) ? 1 : 0;
        long long noise = decoded - expected;

        std::cout << "  " << i << "     | " << expected << "        | "
                  << decoded << "       | " << noise << "\n";

        noise_growth.push_back(noise);
        current = nand_op(current, current);
    }

    // ============================================
    // ANALISIS: Linear o Exponential?
    // ============================================
    std::cout << "\nNOISE GROWTH ANALYSIS:\n";
    std::cout << "-----------------------------------\n";

    bool is_exponential = false;
    for (int i = 2; i < noise_growth.size(); i++) {
        long long prev = noise_growth[i-1];
        long long curr = noise_growth[i];
        if (prev == 0) continue;

        double ratio = (double)curr / prev;
        if (i <= 10) {
            std::cout << "  Step " << i << ": " << prev << " → " << curr
                      << " (ratio=" << ratio << ")\n";
        }
        if (ratio > 2.0 && ratio < 100.0) is_exponential = true;
    }

    std::cout << "\n  Growth type: "
              << (is_exponential ? "EXPONENTIAL" : "BOUNDED/LINEAR")
              << "\n";

    // ============================================
    // TEST 4: NOISE REFRESH — Period-2 ba talaga?
    // ============================================
    std::cout << "\nTEST 4: PERIOD-2 NOISE REFRESH\n";
    std::cout << "-----------------------------------\n";
    std::cout << "  NOT(NOT(x)) = x ba sa noise level?\n";

    auto not_op = [&](NTL::ZZ x) { return nand_op(x, x); };

    // Start with noise e=1
    auto x = encrypt(1, NTL::to_ZZ(1));
    auto not1 = not_op(x);
    auto not2 = not_op(not1);

    long long dec_x = NTL::to_long(decode(x));
    long long dec_not1 = NTL::to_long(decode(not1));
    long long dec_not2 = NTL::to_long(decode(not2));

    std::cout << "  x     = " << dec_x << " (expected 1, noise=" << dec_x - 1 << ")\n";
    std::cout << "  NOT(x) = " << dec_not1 << " (expected 0, noise=" << dec_not1 << ")\n";
    std::cout << "  NOT(NOT(x)) = " << dec_not2 << " (expected 1, noise=" << dec_not2 - 1 << ")\n\n";

    if (dec_not2 == 1 && dec_x - 1 == 1) {
        std::cout << "  RESULT: PERFECT PERIOD-2 — noise did not grow!\n";
    } else {
        std::cout << "  RESULT: Noise changed from " << dec_x - 1
                  << " to " << dec_not2 - 1 << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY FINDING:\n";
    std::cout << "  - Kung noise linear: self-limiting ✓\n";
    std::cout << "  - Kung noise exponential: kailangan bootstrap\n";
    std::cout << "========================================\n";

    return 0;
}
