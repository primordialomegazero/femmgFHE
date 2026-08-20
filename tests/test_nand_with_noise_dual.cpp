// NAND WITH INITIAL NOISE — DUAL DECODE
// Ano ang mangyayari sa ψ-decode kapag may initial noise?

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND WITH INITIAL NOISE — DUAL DECODE\n";
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

    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };

    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    std::cout << "OFFSET = " << offset << "\n\n";

    // Test with different initial noise levels
    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
        std::cout << "INITIAL NOISE e = " << e << "\n";
        std::cout << "-------------------\n";

        // Start with m=1 (true)
        auto current = encrypt(1, e);

        std::cout << "  Depth | φ-decode | ψ-decode | Binary(ψ)?\n";
        std::cout << "  ------|----------|----------|----------\n";

        for (int i = 0; i <= 10; i++) {
            NTL::ZZ dec_phi = decode_phi(current);
            NTL::ZZ dec_psi = decode_psi(current);

            // Check: ang ψ-decode ba ay nasa {0, offset}?
            bool is_binary = (dec_psi == 0 || dec_psi == offset);

            std::cout << "  " << i << "     | " << dec_phi
                      << " | " << dec_psi
                      << " | " << (is_binary ? "YES" : "NO") << "\n";

            current = nand_op(current, current);
        }
        std::cout << "\n";
    }

    // ============================================
    // KEY QUESTION: Ang ψ-decode ba ay nagbibigay
    // ng automatic noise cancellation?
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  KEY QUESTION:\n";
    std::cout << "  Ang ψ-decode ba ay self-correcting?\n";
    std::cout << "========================================\n\n";

    // Test: e=1, deep chain
    auto current = encrypt(1, NTL::to_ZZ(1));
    std::cout << "  e=1, 20 depths:\n";
    std::cout << "  Depth | ψ-decode | Dist to 0 | Dist to offset | Decision\n";
    std::cout << "  ------|----------|-----------|----------------|---------\n";

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ dec_psi = decode_psi(current);
        NTL::ZZ dist_0 = (dec_psi < Q/2) ? dec_psi : Q - dec_psi;
        NTL::ZZ diff = (dec_psi > offset) ? dec_psi - offset : offset - dec_psi;
        NTL::ZZ dist_off = (diff < Q/2) ? diff : Q - diff;

        int decision = (dist_off < dist_0) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        std::cout << "  " << i << "     | " << dec_psi
                  << " | " << dist_0
                  << " | " << dist_off
                  << " | " << decision
                  << (decision == expected ? " ✓" : " ✗") << "\n";

        current = nand_op(current, current);
    }

    return 0;
}
