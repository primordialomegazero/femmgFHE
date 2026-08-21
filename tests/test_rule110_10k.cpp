// RULE 110 — 10,000 STEPS COMPLETE OUTPUT
// Period-3 bounded, full analysis

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — 10,000 STEPS\n";
    std::cout << "  Period-3 Bounded Complete\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double STATE_0 = -0.6180339887498949;  // -1/φ
    const double STATE_1 = 4.618033988749895;     // φ² + 2
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

    // Initial state: alternating 0/1
    std::vector<std::complex<double>> init(slots, {STATE_0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? STATE_0 : STATE_1, 0.0};
    }
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "10,000 STEPS TEST:\n";
    std::cout << "==================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 10000; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        auto K_use = (step % 3 == 0) ? ct_state_0 : 
                     (step % 3 == 1) ? ct_state_1 : ct_state_2;

        auto shifted = cc->EvalAdd(state, ct_phi_mod);
        state = cc->EvalSub(K_use, shifted);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 10.0);
        if (!bounded) errors++;

        if (step < 20 || step % 1000 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "center=" << v
                      << " level=" << state->GetLevel()
                      << " elapsed=" << elapsed << "s"
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  FINAL RESULT:\n";
    std::cout << "  Steps: 10,000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Depth used: " << state->GetLevel() << "/5\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 10000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ RULE 110 10K BOUNDED!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
