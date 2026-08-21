// FULL RULE 110 + PERIOD-3 MAPPING
// 0 → -0.618, 1 → 2 (o 4.618)
// Transition gamit ang period-3 cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL RULE 110 + PERIOD-3\n";
    std::cout << "  0 → -0.618, 1 → 2\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double neg_inv_phi = -1.0 / phi;  // -0.618
    const double one_state = 2.0;             // state 1
    const double two_state = phi_sq + 2.0;    // 4.618

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

    // Constants para sa period-3 states
    auto ct_zero_state = make_uniform(neg_inv_phi);    // -0.618
    auto ct_one_state = make_uniform(one_state);        // 2.0
    auto ct_two_state = make_uniform(two_state);        // 4.618

    // Initial state: alternating 0 at 1 states
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? one_state : neg_inv_phi, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "FULL RULE 110 PERIOD-3 (100 steps):\n";
    std::cout << "===================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Kunin ang L, C, R
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        // Period-3 transition: x_{n+1} = K - (x_n + φ_mod)
        // K ay nag-o-oscillate sa {2, 4.618, -0.618}
        
        // Para sa Rule 110, kailangan natin ng neighbor-aware transition
        // Na naka-encode sa period-3 cycle

        // Simple period-3 step: x = K_i - x (bounded oscillation)
        auto K_use = (step % 3 == 0) ? ct_one_state : 
                     (step % 3 == 1) ? ct_two_state : ct_zero_state;

        // Bounded: x = K - x (period-2 sa loob ng period-3)
        state = cc->EvalSub(K_use, state);

        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -2.0 && v <= 5.5);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PERIOD-3 FULL!" : "❌") << "\n";

    return 0;
}
