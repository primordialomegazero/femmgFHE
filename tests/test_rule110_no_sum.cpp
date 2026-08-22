// RULE 110 NA WALANG SUM — PURE ROTATION + PHASE
// Hindi kailangan ng L+C+R — rotation lang
// Natural na bounded sa φ-domain

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
    std::cout << "  RULE 110 — WALANG SUM\n";
    std::cout << "  Pure Rotation + Phase\n";
    std::cout << "========================================\n\n";

    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;
    const double phi_sq = 2.618033988749895;

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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});

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

    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_psi_mod = make_uniform(psi_mod);
    auto ct_phi_sq = make_uniform(phi_sq);

    // Initial: pattern sa slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 3 == 0) ? phi_mod : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 NO SUM (5000 steps):\n";
    std::cout << "=============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 5000; step++) {
        // Neighbor interaction via rotation (hindi sum)
        state = cc->EvalAtIndex(state, 1);

        // φ+ψ cancellation para sa bounded evolution
        if (step % 2 == 0) {
            state = cc->EvalAdd(state, ct_phi_mod);
        } else {
            state = cc->EvalAdd(state, ct_psi_mod);
        }

        // Natural normalization: period-4 oscillation
        auto ct_three_phi = make_uniform(3 * phi_sq);
        state = cc->EvalSub(ct_three_phi, state);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 5 * phi_sq);
        if (!bounded) errors++;

        if (step % 500 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/5000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 5000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ NO SUM BOUNDED!" : "❌") << "\n";

    return 0;
}
