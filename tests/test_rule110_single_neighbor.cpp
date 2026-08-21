// RULE 110 — SINGLE NEIGHBOR BOUNDED
// φ² - L (single left neighbor) — walang growth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — SINGLE NEIGHBOR\n";
    std::cout << "  φ² - L (bounded)\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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

    std::vector<std::complex<double>> phi_sq_vec(slots, {phi_sq, 0.0});
    auto ct_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_sq_vec));

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "SINGLE NEIGHBOR 1000 STEPS:\n";
    std::cout << "===========================\n\n";

    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        state = cc->EvalSub(ct_phi_sq, left);

        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -0.1 && v <= phi_sq + 0.1);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            double v128 = decrypt_slot(state, 128);
            double v0 = decrypt_slot(state, 0);
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v128
                      << " slot0=" << v0
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ SINGLE NEIGHBOR BOUNDED!" : "❌") << "\n";

    return 0;
}
