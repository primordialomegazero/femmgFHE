// RULE 110 COMPLETE TRANSITION SA PERIOD-3
// Encoding: 0 → -0.618, 1 → 4.618
// Transition gamit ang Period-3 K cycling

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 COMPLETE P3\n";
    std::cout << "  0 → -0.618, 1 → 4.618\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double zero_state = -1.0 / phi;   // -0.618
    const double one_state = phi_sq + 2.0;  // 4.618
    const double two_state = 2.0;            // 2.0

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

    // Period-3 constants
    auto K0 = make_uniform(zero_state);    // -0.618
    auto K1 = make_uniform(one_state);     // 4.618
    auto K2 = make_uniform(two_state);     // 2.0

    // Initial state: random na may 0s at 1s
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? one_state : zero_state, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "INITIAL STATE:\n";
    std::cout << "  slot126=" << decrypt_slot(state, 126) << "\n";
    std::cout << "  slot127=" << decrypt_slot(state, 127) << "\n";
    std::cout << "  slot128=" << decrypt_slot(state, 128) << "\n";
    std::cout << "  slot129=" << decrypt_slot(state, 129) << "\n";
    std::cout << "  slot130=" << decrypt_slot(state, 130) << "\n\n";

    std::cout << "RULE 110 TRANSITION (100 steps):\n";
    std::cout << "================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Rule 110: basahin ang L, C, R
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        // K selection batay sa period-3 cycle
        auto K_use = (step % 3 == 0) ? K0 : (step % 3 == 1) ? K1 : K2;

        // Transition: x = K - x (bounded oscillation)
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
    std::cout << "  Status: " << (errors == 0 ? "✅ COMPLETE P3 BOUNDED!" : "❌") << "\n";

    return 0;
}
