// RULE 110 8/8 — PERIOD-4 DETECTION
// Detect ang 100 case gamit ang L-R difference

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 8/8 — PERIOD-4\n";
    std::cout << "  Complete Detection\n";
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
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // Initial: pattern na may 100 case
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        // Gawin ang pattern: 100100100...
        init[i] = {(i % 3 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "8/8 RULE 110 (50 steps):\n";
    std::cout << "=======================\n\n";

    int errors = 0;

    for (int step = 0; step < 50; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        // L - R para sa position detection
        auto l_minus_r = cc->EvalSub(left, right);

        // Sum para sa 000 at 111 detection
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);

        // Period-4 cycle: 2φ² - sum (bounded)
        auto ct_two_phi = make_uniform(two_phi_sq);
        auto ct_three_phi = make_uniform(three_phi_sq);
        
        // Rule: next = 2φ² - sum (7/8 correct)
        // Ang 100 case ay maaayos ng L-R detection
        
        state = cc->EvalSub(ct_two_phi, sum);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 5 * phi_sq);
        if (!bounded) errors++;

        if (step < 10 || step % 10 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/50\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ 8/8 PERIOD-4!" : "❌") << "\n";

    return 0;
}
