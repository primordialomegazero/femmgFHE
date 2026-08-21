// RULE 110 + PERIOD-8 MAP
// f(x) = 2φ² - x para sa bounded neighbors

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + PERIOD-8 MAP\n";
    std::cout << "  Bounded Neighbor Interaction\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

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

    std::vector<std::complex<double>> two_phi_vec(slots, {two_phi_sq, 0.0});
    auto ct_two_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_vec));

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PERIOD-8 MAP INTEGRATED (100 steps):\n";
    std::cout << "====================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Neighbor sum na naka-bound ng Period-8 map
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        // Period-8 bounded sum: 2φ² - (L + R)
        auto bounded_sum = cc->EvalSub(ct_two_phi, cc->EvalAdd(left, right));

        // State update: 2φ² - bounded_sum
        state = cc->EvalSub(ct_two_phi, bounded_sum);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            double v128 = decrypt_slot(state, 128);
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v128
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ P8 INTEGRATED BOUNDED!" : "❌") << "\n";

    return 0;
}
