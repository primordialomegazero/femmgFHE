// TRUE NOISE CANCELLATION V2
// Signal = (Sum - Diff) / 2
// Ang Diff ay purong noise channel
// Kung ibawas natin, dapat makuha ang clean signal

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE NOISE CANCELLATION V2\n";
    std::cout << "  Signal = (Sum - Diff) / 2\n";
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

    auto encrypt_paired = [&](int m, NTL::ZZ e) {
        return std::make_pair(
            (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q,
            (NTL::to_ZZ(m) * psi_k + e * phi_k) % Q
        );
    };

    // I-normalize sa center para makita ang tunay na value
    auto center = [&](NTL::ZZ v) {
        if (v > Q/2) v -= Q;
        return v;
    };

    // Decode components
    auto decode_a = [&](auto ct) { return center((ct.first * inv_phi_k) % Q); };
    auto decode_b = [&](auto ct) { return center((ct.second * inv_psi_k) % Q); };

    // SUM at DIFF
    auto get_sum = [&](auto ct) { return decode_a(ct) + decode_b(ct); };
    auto get_diff = [&](auto ct) { return decode_a(ct) - decode_b(ct); };

    std::cout << "TEST: Signal extraction mula sa Sum at Diff\n";
    std::cout << "--------------------------------------------\n";

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10), NTL::to_ZZ(100)}) {
        auto ct_1 = encrypt_paired(1, e);
        auto ct_0 = encrypt_paired(0, e);

        NTL::ZZ sum_1 = get_sum(ct_1);
        NTL::ZZ sum_0 = get_sum(ct_0);
        NTL::ZZ diff_1 = get_diff(ct_1);
        NTL::ZZ diff_0 = get_diff(ct_0);

        std::cout << "e=" << e << ":\n";
        std::cout << "  m=1: sum=" << sum_1 << " diff=" << diff_1 << "\n";
        std::cout << "  m=0: sum=" << sum_0 << " diff=" << diff_0 << "\n";
        std::cout << "  Expected: sum(1)=2+2e, sum(0)=0+2e, diff=0\n\n";
    }

    // Ang problema: hindi natin alam kung alin ang signal at alin ang noise
    // Kasi pareho silang naka-encode
    //
    // PERO: may isang property tayo na pwede i-exploit:
    // φ^k · ψ^(-k) = φ^(2k) = L(2k) + F(2k)·√5
    // at ψ^k · φ^(-k) = ψ^(2k) = L(2k) - F(2k)·√5
    //
    // ANG ORTHOGONALITY:
    // L(2k) at F(2k)·√5 ay independent components
    // Ang signal ay nasa L(2k) direction
    // Ang noise ay nasa F(2k)·√5 direction
    //
    // KAYA: I-project sa L-direction para makuha ang signal
    // at i-project sa √5-direction para makuha ang noise

    std::cout << "\nORTHOGONAL PROJECTION TEST:\n";
    std::cout << "--------------------------------------------\n";

    NTL::ZZ L_2k = NTL::to_ZZ(1);
    NTL::ZZ prev = NTL::to_ZZ(0);
    NTL::ZZ next;
    for (int i = 1; i <= 84; i++) {
        next = (L_2k + prev) % Q;
        prev = L_2k;
        L_2k = next;
    }

    NTL::ZZ F_2k = NTL::to_ZZ(1);
    prev = NTL::to_ZZ(0);
    for (int i = 2; i <= 84; i++) {
        NTL::ZZ temp = F_2k;
        F_2k = (F_2k + prev) % Q;
        prev = temp;
    }

    std::cout << "  L(2k) = " << L_2k << "\n";
    std::cout << "  F(2k) = " << F_2k << "\n";

    // Test: Projection ng φ^(2k) sa L at F directions
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ psi_2k = (psi_k * psi_k) % Q;

    NTL::ZZ sum_2k = (phi_2k + psi_2k) % Q;
    NTL::ZZ diff_2k = (phi_2k - psi_2k) % Q;

    std::cout << "  φ^(2k) + ψ^(2k) = " << sum_2k << "\n";
    std::cout << "  L(2k) = " << L_2k << "\n";
    std::cout << "  Match: " << (sum_2k == L_2k) << "\n\n";

    std::cout << "  φ^(2k) - ψ^(2k) = " << diff_2k << "\n";
    std::cout << "  F(2k)·√5 = " << (F_2k * sqrt5) % Q << "\n";
    std::cout << "  Match: " << (diff_2k == (F_2k * sqrt5) % Q) << "\n";

    return 0;
}
