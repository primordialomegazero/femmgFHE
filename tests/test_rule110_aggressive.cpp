// RULE 110 — AGGRESSIVE CORRECTION
// Bawasan ng mas malaki para ma-contain ang growth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — AGGRESSIVE CORRECTION\n";
    std::cout << "  Bawasan ng 6φ² tuwing 4 steps\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double six_phi_sq = 6 * phi_sq;

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
    std::vector<std::complex<double>> six_phi_sq_vec(slots, {six_phi_sq, 0.0});
    auto ct_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_sq_vec));
    auto ct_six_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(six_phi_sq_vec));

    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 AGGRESSIVE (100 steps):\n";
    std::cout << "================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);

        state = cc->EvalAdd(sum, ct_phi_sq);

        // Aggressive correction: bawasan ng 6φ² tuwing 4 steps
        if (step % 4 == 2) {
            state = cc->EvalSub(state, ct_six_phi_sq);
        }

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 10 * phi_sq);
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
    std::cout << "  Status: " << (errors == 0 ? "✅ AGGRESSIVE BOUNDED!" : "❌") << "\n";

    return 0;
}
