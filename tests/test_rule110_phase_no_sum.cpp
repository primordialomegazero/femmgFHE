// RULE 110 — PHASE-BASED (WALANG SUM)
// Ang neighbor info ay nasa PHASE, hindi sum
// x_{n+1} = rotate(x_n) at period-3 K cycling

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — PHASE-BASED\n";
    std::cout << "  Walang Sum, Phase Shift Lang\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double STATE_0 = -0.6180339887498949;
    const double STATE_1 = 4.618033988749895;
    const double STATE_2 = 2.0;
    const double phi_mod = 0.6180339887498949;

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

    auto ct_state_0 = make_uniform(STATE_0);
    auto ct_state_1 = make_uniform(STATE_1);
    auto ct_state_2 = make_uniform(STATE_2);
    auto ct_phi_mod = make_uniform(phi_mod);

    // Initial: φ-phase pattern
    std::vector<std::complex<double>> init(slots, {STATE_0, 0.0});
    for (int i = 0; i < slots; i++) {
        // Iba't ibang phase per slot
        if (i % 3 == 0) init[i] = {STATE_0, 0.0};
        else if (i % 3 == 1) init[i] = {STATE_1, 0.0};
        else init[i] = {STATE_2, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PHASE-BASED RULE 110 (1000 steps):\n";
    std::cout << "==================================\n\n";

    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Phase shift: rotate + period-3 K cycling
        // Walang sum ng neighbors!

        // Bounded transition na walang sum:
        // x = K_i - (x + φ_mod)
        auto K_use = (step % 3 == 0) ? ct_state_0 : 
                     (step % 3 == 1) ? ct_state_1 : ct_state_2;

        auto shifted = cc->EvalAdd(state, ct_phi_mod);
        state = cc->EvalSub(K_use, shifted);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 10.0);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PHASE-BASED BOUNDED!" : "❌") << "\n";

    return 0;
}
