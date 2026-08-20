// CONJUGATE NOISE ORACLE — PROOF OF CONCEPT
// Gamit ang std_noise - masked para i-extract ang noise
// Pagkatapos: clean_signal = std_noise - noise

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  CONJUGATE NOISE ORACLE\n";
    std::cout << "  Hahanapin: ang noise oracle ba?\n";
    std::cout << "========================================\n\n";

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
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);

    auto center = [&](NTL::ZZ v) {
        if (v > Q/2) v -= Q;
        return v;
    };

    // ============================================
    // BASIC ENCRYPTION: ct = m·φ^k + e·ψ^k
    // ============================================
    auto encrypt_std = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    // ============================================
    // NOISE ORACLE: std_decode - masked_decode
    // ============================================
    // std_decode = ct·φ^(-k) = m + e·ψ^k·φ^(-k) = m + e  (k=42 even)
    // masked = ct·ψ^k
    // masked_decode = masked·φ^(-k) = ct·ψ^k·φ^(-k)
    //              = (m·φ^k + e·ψ^k)·ψ^k·φ^(-k)
    //              = m·ψ^k·φ^(k-k) + e·ψ^(2k)·φ^(-k)
    //              = m·1 + e·ψ^(2k)·φ^(-k)
    //              = m + e·L(2k)·ψ^k·φ^(-k) - e·φ^(-k)
    //              = m + e·L(2k) - e·φ^(-k)  [k=42 even, ψ^k·φ^(-k)=1]
    //
    // DIFFERENCE: std_decode - masked_decode
    // = (m + e) - (m + e·L(2k) - e·φ^(-k))
    // = e - e·L(2k) + e·φ^(-k)
    // = e·(1 - L(2k) + φ^(-k))
    //
    // CONSTANT para sa given e!
    // Kaya ang difference ay INDEPENDENT sa message m

    auto decode_std = [&](NTL::ZZ ct) {
        return center((ct * inv_phi_k) % Q);
    };

    auto decode_masked = [&](NTL::ZZ ct) {
        NTL::ZZ masked = (ct * psi_k) % Q;
        return center((masked * inv_phi_k) % Q);
    };

    auto noise_oracle = [&](NTL::ZZ ct) {
        return decode_std(ct) - decode_masked(ct);
    };

    // ============================================
    // TEST 1: NOISE ORACLE CONSISTENCY
    // ============================================
    std::cout << "TEST 1: NOISE ORACLE CONSISTENCY\n";
    std::cout << "----------------------------------\n";
    std::cout << "  Ang noise oracle ba ay pareho para sa m=0 at m=1?\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10), NTL::to_ZZ(100)}) {
        auto ct_0 = encrypt_std(0, e);
        auto ct_1 = encrypt_std(1, e);

        NTL::ZZ oracle_0 = noise_oracle(ct_0);
        NTL::ZZ oracle_1 = noise_oracle(ct_1);

        std::cout << "  e=" << e << ": oracle(m=0)=" << oracle_0
                  << " oracle(m=1)=" << oracle_1;
        if (oracle_0 == oracle_1) {
            std::cout << " ✓ CONSISTENT";
        } else {
            std::cout << " ✗ DIFFERENT";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // ============================================
    // TEST 2: NOISE EXTRACTION
    // ============================================
    std::cout << "TEST 2: NOISE EXTRACTION\n";
    std::cout << "----------------------------------\n";
    std::cout << "  Makukuha ba natin ang exact noise value?\n\n";

    auto extract_noise = [&](NTL::ZZ ct) {
        // noise = std_decode - m
        // Pero hindi natin alam ang m...
        // GAMIT ANG ORACLE: ang oracle ay independent sa m
        // Kaya: oracle = e·(1 - L(2k) + φ^(-k))
        // Kung alam natin ang (1 - L(2k) + φ^(-k)),
        // pwede nating i-recover ang e

        NTL::ZZ oracle_val = noise_oracle(ct);
        return oracle_val;
    };

    // Compute ang factor
    NTL::ZZ L_2k;
    NTL::ZZ L_prev = NTL::to_ZZ(2);
    NTL::ZZ L_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 84; i++) {
        NTL::ZZ temp = L_curr;
        L_curr = (L_curr + L_prev) % Q;
        L_prev = temp;
    }
    L_2k = L_curr;

    NTL::ZZ factor = (NTL::to_ZZ(1) - L_2k + inv_phi_k) % Q;
    if (factor < 0) factor += Q;

    std::cout << "  L(2k) = " << L_2k << "\n";
    std::cout << "  factor = 1 - L(2k) + φ^(-k) = " << factor << "\n\n";

    // Subukan i-recover ang e
    NTL::ZZ inv_factor = NTL::InvMod(factor, Q);

    std::cout << "  NOISE RECOVERY:\n";
    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        auto ct = encrypt_std(1, e);
        NTL::ZZ oracle_val = noise_oracle(ct);
        NTL::ZZ recovered_e = center((oracle_val * inv_factor) % Q);

        std::cout << "    e=" << e << " → recovered=" << recovered_e;
        if (recovered_e == e) {
            std::cout << " ✓ EXACT";
        } else {
            std::cout << " ✗ MISMATCH";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // ============================================
    // TEST 3: NAND NOISE TRACKING
    // ============================================
    std::cout << "TEST 3: NAND NOISE TRACKING\n";
    std::cout << "----------------------------------\n";

    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto current = encrypt_std(1, NTL::to_ZZ(1));
    std::cout << "  Depth | std_noise | oracle | recovered_e | clean?\n";
    std::cout << "  ------|-----------|--------|-------------|-------\n";

    for (int i = 0; i <= 10; i++) {
        NTL::ZZ decoded = decode_std(current);
        NTL::ZZ expected_val = (i % 2 == 0) ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        NTL::ZZ std_noise = decoded - expected_val;

        NTL::ZZ oracle_val = noise_oracle(current);
        NTL::ZZ recovered_e = center((oracle_val * inv_factor) % Q);

        // Clean signal: decoded - noise = decoded - recovered_e
        NTL::ZZ clean = decoded - recovered_e;
        NTL::ZZ clean_expected = (i % 2 == 0) ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        bool is_clean = (center(clean) == clean_expected);

        std::cout << "  " << i << "     | " << std_noise
                  << " | " << oracle_val
                  << " | " << recovered_e
                  << " | " << (is_clean ? "✓" : "✗") << "\n";

        current = nand_op(current, current);
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Kung may ✓ sa clean column,\n";
    std::cout << "    ang conjugate oracle ay gumagana!\n";
    std::cout << "========================================\n";

    return 0;
}
