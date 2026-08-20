// DUAL DECODE NAND — TRUE NOISE CANCELLATION VERIFICATION
// ψ-decode = e (noise) para sa m=0
// ψ-decode = offset + e (signal + noise) para sa m=1
// Kaya pwede nating ihiwalay ang signal sa noise

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  DUAL DECODE NAND CHAIN\n";
    std::cout << "  True Noise Cancellation Verification\n";
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

    // Precompute ang offset
    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    // Encrypt: ct = m·φ^k + e·ψ^k
    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    // Dual decode: φ-decode at ψ-decode
    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };

    // NAND operation
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    std::cout << "OFFSET = φ^k·ψ^(-k) = " << offset << "\n\n";

    // ============================================
    // TEST 1: DECODE EXACTNESS
    // ============================================
    std::cout << "TEST 1: DECODE EXACTNESS\n";
    std::cout << "-------------------------\n";

    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt(m, e);
            NTL::ZZ dec_phi = decode_phi(ct);
            NTL::ZZ dec_psi = decode_psi(ct);

            std::cout << "  m=" << m << " e=" << e << "\n";
            std::cout << "    φ-decode = " << dec_phi << "\n";
            std::cout << "    ψ-decode = " << dec_psi << "\n";

            // Check: ψ-decode = m·offset + e (hindi modulo)
            NTL::ZZ expected_psi = (NTL::to_ZZ(m) * offset + e) % Q;
            std::cout << "    expected ψ-decode = " << expected_psi;
            if (dec_psi == expected_psi) {
                std::cout << " ✓";
            } else {
                std::cout << " ✗";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // TEST 2: NOISE EXTRACTION
    // ============================================
    std::cout << "TEST 2: NOISE EXTRACTION\n";
    std::cout << "-------------------------\n";
    std::cout << "  noise = ψ-decode - m·offset\n";
    std::cout << "  (Kung alam natin ang m, makukuha ang e)\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        auto ct_0 = encrypt(0, e);
        auto ct_1 = encrypt(1, e);

        NTL::ZZ psi_0 = decode_psi(ct_0);
        NTL::ZZ psi_1 = decode_psi(ct_1);

        // m=0: noise = ψ-decode
        // m=1: noise = ψ-decode - offset
        NTL::ZZ noise_0 = psi_0;
        NTL::ZZ noise_1 = (psi_1 - offset + Q) % Q;

        std::cout << "  e=" << e << "\n";
        std::cout << "    noise(m=0) = " << noise_0 << "\n";
        std::cout << "    noise(m=1) = " << noise_1 << "\n";
    }
    std::cout << "\n";

    // ============================================
    // TEST 3: NAND CHAIN WITH DUAL DECODE
    // ============================================
    std::cout << "TEST 3: NAND CHAIN WITH DUAL DECODE\n";
    std::cout << "-----------------------------------\n";
    std::cout << "  Subaybayan ang signal at noise sa bawat depth\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));  // m=1, e=1

    std::cout << "  Depth | φ-decode | ψ-decode | ψ-offset | m=ψ-offset? | e=ψ-m·off?\n";
    std::cout << "  ------|----------|----------|----------|-------------|------------\n";

    for (int i = 0; i <= 15; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        // Subukang i-recover ang m at e
        // m=0: ψ-decode ≈ 0 (dapat small)
        // m=1: ψ-decode ≈ offset (dapat malapit sa offset)
        
        NTL::ZZ dist_to_0 = dec_psi;
        if (dist_to_0 > Q/2) dist_to_0 = Q - dist_to_0;
        
        NTL::ZZ dist_to_offset = (dec_psi > offset) ? dec_psi - offset : offset - dec_psi;
        if (dist_to_offset > Q/2) dist_to_offset = Q - dist_to_offset;

        int recovered_m = (dist_to_offset < dist_to_0) ? 1 : 0;
        NTL::ZZ recovered_e = (recovered_m == 1) ? 
            ((dec_psi - offset + Q) % Q) : dec_psi;

        int expected_m = (i % 2 == 0) ? 1 : 0;

        std::cout << "  " << i << "     | "
                  << (dec_phi < Q/2 ? "small" : "large") << " | "
                  << (dec_psi < Q/2 ? "small" : "large") << " | "
                  << (dist_to_offset < dist_to_0 ? "offset" : "zero") << " | "
                  << recovered_m << " | "
                  << recovered_e << "\n";

        current = nand_op(current, current);
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Nakikita ba natin ang period-2?\n";
    std::cout << "  - Ang noise ba ay na-se-separate?\n";
    std::cout << "========================================\n";

    return 0;
}
