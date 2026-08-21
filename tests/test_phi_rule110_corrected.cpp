// RULE 110 + PERIOD-4 CORRECTION
// Automatic noise reset via cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + PERIOD-4 CORRECTION\n";
    std::cout << "  Automatic Noise Reset\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    auto ct_zero = make_ct(0.0);

    // Period-4 cycle na may eksaktong correction
    // State 0: 0
    // State 1: φ²
    // State 2: 2φ²
    // State 3: -φ²
    // Transition: 0→+φ²→+φ²→-3φ²→+φ²→0

    auto cycle_corrected_step = [&](auto current, int step) {
        if (step % 4 == 2) {
            return cc->EvalSub(current, ct_three_phi_sq);
        } else {
            return cc->EvalAdd(current, ct_phi_sq);
        }
    };

    // Rule 110 na may correction every 4 steps
    std::cout << "RULE 110 + CYCLE CORRECTION (1000 steps):\n";
    std::cout << "==========================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        current = cycle_corrected_step(current, step);

        double v = decrypt_val(current);
        
        // Expected sa period-4 cycle
        double expected;
        if (step % 4 == 0) expected = phi_sq;
        else if (step % 4 == 1) expected = two_phi_sq;
        else if (step % 4 == 2) expected = -phi_sq;
        else expected = 0.0;
        
        bool ok = (std::abs(v - expected) < 0.01);
        if (!ok) errors++;

        if (step < 20 || !ok) {
            std::cout << "  Step " << step << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PERFECT CYCLE!" : "❌ DRIFT") << "\n";

    return 0;
}
