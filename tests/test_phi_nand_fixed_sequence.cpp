// NAND BILANG FIXED CYCLE SEQUENCE
// Ang NAND ay naka-encode bilang sequence ng cycle steps
// para automatic bounded

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND AS FIXED CYCLE SEQUENCE\n";
    std::cout << "  Automatic Boundedness\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // NAND AS SEQUENCE OF CYCLE OPERATIONS
    // ============================================
    // Sa cycle: 0 → φ² → 2φ² → -φ² → 0
    //
    // NAND(0,0) = 2φ²: cycle steps: 0 → +φ² → +φ² = 2φ²
    // NAND(0,φ²) = φ²: cycle steps: 0 → +φ² = φ²
    // NAND(φ²,φ²) = 0: cycle steps: φ² → -φ² → +φ² = 0
    //
    // Ang NAND ay maaaring i-express bilang:
    //   NAND(a,b) = (a + b) mod cycle
    //
    // Sa cycle space, ang addition ay modulo 4:
    //   (state_a + state_b) % 4 = state_nand

    std::cout << "NAND SA CYCLE SPACE:\n";
    std::cout << "====================\n\n";

    // Sa cycle: 0=0, φ²=1, 2φ²=2, -φ²=3
    // NAND states: (a+b) % 4
    // NAND(0,0) = (0+0)%4 = 0 (state 0 = 0) — dapat 2φ²
    // Hindi ito match. Kailangan ng ibang mapping.

    // Alternatibo: NAND = (a+b+2) % 4
    // NAND(0,0) = (0+0+2)%4 = 2 → 2φ² ✓
    // NAND(0,1) = (0+1+2)%4 = 3 → -φ² (dapat φ²)
    // Hindi pa rin match.

    // Subukan: NAND = (a+b+1) % 4
    // NAND(0,0) = 1 → φ² (dapat 2φ²)
    // Hindi match.

    // Ang totoong mapping ay dependent sa state values.
    // Sa φ²-domain:
    //   state 0 (0): NAND(0,0) = 2φ² = state 2
    //   state 1 (φ²): NAND(0,φ²) = φ² = state 1
    //   state 2 (2φ²): NAND(φ²,φ²) = 0 = state 0
    //
    // Ito ay: NAND_state(a,b) = (2 - a - b) % 4

    auto nand_cycle = [&](auto a, auto b) {
        // NAND_state = (2 - a - b) mod 4
        // Sa values: 2φ² - (a+b) na may mod 4φ²
        auto sum = cc->EvalAdd(a, b);
        auto nand_val = cc->EvalSub(make_ct(two_phi_sq), sum);
        return nand_val;
    };

    std::cout << "CYCLE NAND TEST (single gates):\n";
    std::cout << "===============================\n\n";

    auto ct_zero = make_ct(0.0);
    auto ct_phi_sq = make_ct(phi_sq);

    auto t00 = nand_cycle(ct_zero, ct_zero);
    auto t0p = nand_cycle(ct_zero, ct_phi_sq);
    auto tpp = nand_cycle(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << two_phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0p) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tpp) << " (expected 0)\n\n";

    // 100 gates na may modulo every 4 steps
    std::cout << "100 GATES — MODULO EVERY 4 STEPS:\n";
    std::cout << "==================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 100; gate++) {
        current = nand_cycle(current, current);
        
        // Modulo: kung lumampas sa 2φ², subtract 4φ²
        // (ito ay approximation ng cycle modulo)
        double v = decrypt_val(current);
        if (std::abs(v) > 3 * phi_sq) {
            current = cc->EvalSub(current, make_ct(4 * phi_sq));
            v = decrypt_val(current);
        }

        if (gate < 10 || std::abs(v) > 3 * phi_sq) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " level=" << current->GetLevel() << "\n";
        }
    }

    return 0;
}
