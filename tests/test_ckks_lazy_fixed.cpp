// CKKS LAZY REFRESH — FIXED
// Mas maingat na noise estimation
// Hindi na level-based — gamitin ang actual CKKS depth
//
// ANG KEY:
// Ang level sa CKKS ay bumababa sa bawat multiplication
// Level 60 = fresh, Level 0 = depleted
// Ang refresh ay dapat sa level ~20 (may margin pa)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS LAZY REFRESH — FIXED\n";
    std::cout << "  Level-Based Refresh\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);  // Mas mababang depth para sa mas malinis na test
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << ", Depth: 30\n\n";

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

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto ct_one = make_ct(1.0);
        return cc->EvalSub(ct_one, prod);
    };

    // φ/ψ refresh constants
    const double psi_10 = std::pow(1.0/1.6180339887498948482, 10.0);
    const double phi_10 = std::pow(1.6180339887498948482, 10.0);
    auto ct_psi10 = make_ct(psi_10);
    auto ct_phi10 = make_ct(phi_10);

    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi10); };
    auto refresh_phi = [&](auto ct) { return cc->EvalMult(ct, ct_phi10); };

    std::cout << "FIXED TEST:\n";
    std::cout << "============\n\n";
    std::cout << "  Strategy: refresh sa level ≤ 10 (may 10 gates buffer)\n";
    std::cout << "  Hindi na threshold-based — level-based\n\n";

    auto current = make_ct(1.0);
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 20;
    int refresh_count = 0;
    int refresh_level = 10;  // Refresh kapag level ≤ 10

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        int level = current->GetLevel();

        // Refresh kapag level ay bumaba sa threshold
        if (level <= refresh_level && gate < total_gates - 1) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
            } else {
                current = refresh_phi(current);
                scale_idx = 0;
            }
            refresh_count++;
            std::cout << "  [Refresh sa gate " << gate << ", level=" << level << "]\n";
        }

        // Verify
        double v = decrypt_val(current);
        double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
        int expected_int = (gate % 2 == 0) ? 0 : 1;
        double expected = expected_int ? scale_factor : 0.0;
        int got = (std::abs(v) > scale_factor * 0.5) ? 1 : 0;

        if (got != expected_int) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << " level=" << level
                  << (got == expected_int ? " ✓" : " ✗") << "\n";
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Refreshes: " << refresh_count << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
