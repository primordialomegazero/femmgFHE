// PAIRED ENCODING — TRUE NOISE CANCELLATION
// ct = (m·φ^k + e·ψ^k, m·ψ^k + e·φ^k)
// Kapag na-combine, ang noise ay pwedeng mag-cancel

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PAIRED ENCODING NOISE CANCELLATION\n";
    std::cout << "  Hahanapin: true noise cancellation\n";
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

    // Lucas number L(k) = φ^k + ψ^k
    NTL::ZZ L_k = (phi_k + psi_k) % Q;

    // ============================================
    // PAIRED ENCODING:
    // ct_a = m·φ^k + e·ψ^k
    // ct_b = m·ψ^k + e·φ^k
    // ============================================

    auto encrypt_paired = [&](int m, NTL::ZZ e) {
        return std::make_pair(
            (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q,
            (NTL::to_ZZ(m) * psi_k + e * phi_k) % Q
        );
    };

    // DECODE METHOD 1: Sum
    // ct_a·φ^(-k) + ct_b·ψ^(-k) = (m + e) + (m + e) = 2m + 2e
    auto decode_sum = [&](auto ct) {
        NTL::ZZ part_a = (ct.first * inv_phi_k) % Q;
        NTL::ZZ part_b = (ct.second * inv_psi_k) % Q;
        return (part_a + part_b) % Q;
    };

    // DECODE METHOD 2: Difference (cancellation!)
    // ct_a·φ^(-k) - ct_b·ψ^(-k) = (m + e) - (m + e) = 0
    // Pero ang signal ay mawawala rin... kailangan i-scale
    auto decode_diff = [&](auto ct) {
        NTL::ZZ part_a = (ct.first * inv_phi_k) % Q;
        NTL::ZZ part_b = (ct.second * inv_psi_k) % Q;
        NTL::ZZ diff = (part_a - part_b) % Q;
        if (diff < 0) diff += Q;
        return diff;
    };

    // DECODE METHOD 3: Weighted sum
    // α·ct_a·φ^(-k) + β·ct_b·ψ^(-k) = α(m+e) + β(m+e) = (α+β)(m+e)
    // Kung α = 1, β = -1 → pure noise cancellation
    // Kung α = 1, β = 1 → pure signal amplification
    // Hindi natin makukuha ang signal ng walang noise

    // Pero may isa pang approach: ANG NOISE AY E·ψ^k
    // ct_a = m·φ^k + e·ψ^k
    // Kung i-multiply natin ng ψ^k:
    // ct_a·ψ^k = m·φ^k·ψ^k + e·ψ^(2k) = m + e·ψ^(2k)
    // ct_b·ψ^k = m·ψ^(2k) + e·φ^k·ψ^k = m·ψ^(2k) + e

    // ============================================
    // TEST: NOISE MEASUREMENT SA PAIRED ENCODING
    // ============================================

    std::cout << "LUCAS L(k) = " << L_k << "\n\n";

    // Perfect: e=0
    auto perfect_1 = encrypt_paired(1, NTL::to_ZZ(0));
    auto perfect_0 = encrypt_paired(0, NTL::to_ZZ(0));

    NTL::ZZ sum_1 = decode_sum(perfect_1);
    NTL::ZZ sum_0 = decode_sum(perfect_0);
    NTL::ZZ diff_1 = decode_diff(perfect_1);
    NTL::ZZ diff_0 = decode_diff(perfect_0);

    std::cout << "PERFECT (e=0):\n";
    std::cout << "  decode_sum(m=1) = " << sum_1 << " (expected 2)\n";
    std::cout << "  decode_sum(m=0) = " << sum_0 << " (expected 0)\n";
    std::cout << "  decode_diff(m=1) = " << diff_1 << " (expected 0)\n";
    std::cout << "  decode_diff(m=0) = " << diff_0 << " (expected 0)\n\n";

    // With noise: e=5
    auto noisy_1 = encrypt_paired(1, NTL::to_ZZ(5));
    auto noisy_0 = encrypt_paired(0, NTL::to_ZZ(5));

    sum_1 = decode_sum(noisy_1);
    sum_0 = decode_sum(noisy_0);
    diff_1 = decode_diff(noisy_1);
    diff_0 = decode_diff(noisy_0);

    std::cout << "NOISY (e=5):\n";
    std::cout << "  decode_sum(m=1) = " << sum_1 << " (expected 2+10=12)\n";
    std::cout << "  decode_sum(m=0) = " << sum_0 << " (expected 0+10=10)\n";
    std::cout << "  decode_diff(m=1) = " << diff_1 << " (expected 0)\n";
    std::cout << "  decode_diff(m=0) = " << diff_0 << " (expected 0)\n\n";

    // ============================================
    // TRUE NOISE CANCELLATION VIA CONJUGATE MASK
    // ============================================
    std::cout << "TRUE NOISE CANCELLATION TEST:\n";
    std::cout << "-----------------------------------\n";

    // Idea: Kung ang noise ay e·ψ^k, i-multiply natin ng φ^k
    // para ang noise ay maging e·(φ^k·ψ^k) = e·(-1)^k
    // = e (constant!)
    // Pagkatapos, i-subtract natin ang constant e

    // Pero hindi natin alam ang e...
    // Kailangan natin ng BLIND extraction

    // Isa pang approach: CONJUGATE PAIRING
    // ct_a = m·φ^k + e·ψ^k
    // ct_b = m·ψ^k + e·φ^k
    //
    // ct_a·φ^(-k) = m + e·ψ^k·φ^(-k) = m + e
    // ct_b·ψ^(-k) = m + e·φ^k·ψ^(-k) = m + e
    //
    // Pareho lang! Ang difference ay 0
    // Pero ang SUM ay 2(m+e)
    // Ang noise ay hindi magse-separate sa signal

    // ANG TOTOONG KEY:
    // φ^k - ψ^k = F(k)·√5 (Fibonacci × √5)
    // Ito ay ORTHOGONAL sa L(k) = φ^k + ψ^k

    NTL::ZZ F_k = NTL::to_ZZ(1);
    NTL::ZZ prev = NTL::to_ZZ(0);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = F_k;
        F_k = (F_k + prev) % Q;
        prev = temp;
    }

    NTL::ZZ diff_phi_psi = (phi_k - psi_k) % Q;
    if (diff_phi_psi < 0) diff_phi_psi += Q;

    std::cout << "  F(42) = " << F_k << "\n";
    std::cout << "  φ^k - ψ^k = " << diff_phi_psi << "\n";
    std::cout << "  F(k)·√5 = " << (F_k * sqrt5) % Q << "\n";
    std::cout << "  Match: " << (diff_phi_psi == (F_k * sqrt5) % Q) << "\n\n";

    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - L(k) = φ^k + ψ^k ay scalar (madaling i-decode)\n";
    std::cout << "  - F(k)·√5 = φ^k - ψ^k ay orthogonal component\n";
    std::cout << "  - Kung ang signal ay nasa L(k) direction\n";
    std::cout << "    at ang noise ay nasa F(k)·√5 direction,\n";
    std::cout << "    sila ay ORTHOGONAL at hindi maghahalo\n";

    return 0;
}
