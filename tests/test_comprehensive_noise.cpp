// COMPREHENSIVE TRUE NOISE CANCELLATION RESEARCH
// Lahat ng approach sabay-sabay

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  COMPREHENSIVE NOISE CANCELLATION\n";
    std::cout << "  5 Approaches Sabay-sabay\n";
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
    NTL::ZZ inv2_Q = NTL::InvMod(NTL::to_ZZ(2), Q);

    auto center = [&](NTL::ZZ v) {
        if (v > Q/2) v -= Q;
        return v;
    };

    // ============================================
    // APPROACH 1: ORTHOGONAL PROJECTION
    // ============================================
    std::cout << "APPROACH 1: ORTHOGONAL PROJECTION\n";
    std::cout << "=================================\n";

    // Tamang Lucas para k=42
    NTL::ZZ L_k;
    NTL::ZZ L_prev = NTL::to_ZZ(2);
    NTL::ZZ L_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = L_curr;
        L_curr = (L_curr + L_prev) % Q;
        L_prev = temp;
    }
    L_k = L_curr;

    // Tamang Fibonacci para k=42
    NTL::ZZ F_k;
    NTL::ZZ F_prev = NTL::to_ZZ(0);
    NTL::ZZ F_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = F_curr;
        F_curr = (F_curr + F_prev) % Q;
        F_prev = temp;
    }
    F_k = F_curr;

    NTL::ZZ D_k = (F_k * sqrt5) % Q;

    std::cout << "  L(42) = " << L_k << "\n";
    std::cout << "  F(42) = " << F_k << "\n";
    std::cout << "  D(k) = F(k)·√5 = " << D_k << "\n";
    std::cout << "  φ^k = (L + D)/2: " << ((((L_k + D_k) % Q) * inv2_Q) % Q == phi_k) << "\n";
    std::cout << "  ψ^k = (L - D)/2: " << ((((L_k - D_k + Q) % Q) * inv2_Q) % Q == psi_k) << "\n\n";

    // Projection functions
    auto project_L = [&](NTL::ZZ ct) {
        return center((ct * NTL::InvMod(L_k, Q)) % Q);
    };
    auto project_F = [&](NTL::ZZ ct) {
        return center((ct * NTL::InvMod(D_k, Q)) % Q);
    };

    // ============================================
    // ENCRYPTION
    // ============================================
    auto encrypt_std = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    std::cout << "  Projection of encrypted values:\n";
    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt_std(m, e);
            std::cout << "    m=" << m << " e=" << e
                      << " | L-proj=" << project_L(ct)
                      << " F-proj=" << project_F(ct) << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // APPROACH 2: PAIRED ENCODING
    // ============================================
    std::cout << "APPROACH 2: PAIRED ENCODING + CORRECTION\n";
    std::cout << "=========================================\n";

    auto encrypt_paired = [&](int m, NTL::ZZ e) {
        return std::make_pair(
            (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q,
            (NTL::to_ZZ(m) * psi_k + e * phi_k) % Q
        );
    };

    auto decode_pair = [&](auto ct) {
        NTL::ZZ a = center((ct.first * inv_phi_k) % Q);
        NTL::ZZ b = center((ct.second * inv_psi_k) % Q);
        return std::make_pair(a, b);
    };

    std::cout << "  Paired decode (a, b):\n";
    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt_paired(m, e);
            auto decoded = decode_pair(ct);
            std::cout << "    m=" << m << " e=" << e
                      << " → (" << decoded.first << ", " << decoded.second << ")\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // APPROACH 3: CONJUGATE MASKING
    // ============================================
    std::cout << "APPROACH 3: CONJUGATE MASKING\n";
    std::cout << "=================================\n";

    auto conjugate_mask = [&](NTL::ZZ ct) {
        return (ct * psi_k) % Q;
    };

    std::cout << "  Conjugate masking (ct · ψ^k):\n";
    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(5)}) {
            auto ct = encrypt_std(m, e);
            auto masked = conjugate_mask(ct);
            std::cout << "    m=" << m << " e=" << e
                      << " → masked=" << center(masked) << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // APPROACH 4: LUCAS NOISE LADDER
    // ============================================
    std::cout << "APPROACH 4: LUCAS NOISE LADDER\n";
    std::cout << "=================================\n";

    std::cout << "  Lucas sequence values:\n";
    NTL::ZZ l0 = NTL::to_ZZ(2);
    NTL::ZZ l1 = NTL::to_ZZ(1);
    for (int i = 0; i <= 10; i++) {
        if (i == 0) std::cout << "    L(0) = " << l0 << "\n";
        else if (i == 1) std::cout << "    L(1) = " << l1 << "\n";
        else {
            NTL::ZZ temp = l1;
            l1 = (l1 + l0) % Q;
            l0 = temp;
            std::cout << "    L(" << i << ") = " << l1 << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // APPROACH 5: FIBONACCI NOISE LADDER
    // ============================================
    std::cout << "APPROACH 5: FIBONACCI NOISE LADDER\n";
    std::cout << "=================================\n";

    std::cout << "  D(k) = F(k)·√5 = " << D_k << "\n";
    std::cout << "  L(k) = " << L_k << "\n";
    std::cout << "  Ratio D/L = " << center((D_k * NTL::InvMod(L_k, Q)) % Q) << "\n\n";

    // ============================================
    // NAND NOISE TRACE
    // ============================================
    std::cout << "NAND NOISE TRACE — ALL APPROACHES\n";
    std::cout << "=================================\n";

    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto current = encrypt_std(1, NTL::to_ZZ(1));
    std::cout << "  Depth | std_noise | proj_L | proj_F | masked\n";
    std::cout << "  ------|-----------|--------|--------|-------\n";

    for (int i = 0; i <= 10; i++) {
        NTL::ZZ decoded = center((current * inv_phi_k) % Q);
        NTL::ZZ expected_val = (i % 2 == 0) ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        NTL::ZZ noise = decoded - expected_val;

        NTL::ZZ pl = project_L(current);
        NTL::ZZ pf = project_F(current);
        NTL::ZZ masked = center(conjugate_mask(current));

        std::cout << "  " << i << "     | " << noise
                  << " | " << pl << " | " << pf << " | " << masked << "\n";

        current = nand_op(current, current);
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY FINDINGS:\n";
    std::cout << "  - Aling approach ang may bounded noise?\n";
    std::cout << "  - Saan may natural cancellation?\n";
    std::cout << "========================================\n";

    return 0;
}
