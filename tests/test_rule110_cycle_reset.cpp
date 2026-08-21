// RULE 110 + EXPLICIT CYCLE RESET
// Tuwing 4 steps, i-reset sa period-4 cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + CYCLE RESET\n";
    std::cout << "  Period-4 Correction tuwing 4 steps\n";
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

    // Constants
    std::vector<std::complex<double>> phi_sq_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> three_phi_sq_vec(slots, {three_phi_sq, 0.0});
    auto ct_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_sq_vec));
    auto ct_three_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(three_phi_sq_vec));

    // Initial state
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 + CYCLE RESET (100 steps):\n";
    std::cout << "===================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Rule 110 neighbor sum
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);

        // Step: +φ² para sa lahat
        state = cc->EvalAdd(sum, ct_phi_sq);

        // Period-4 correction tuwing 4 steps
        if (step % 4 == 2) {
            state = cc->EvalSub(state, ct_three_phi_sq);
        }

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 4 * phi_sq);
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
    std::cout << "  Status: " << (errors == 0 ? "✅ CYCLE RESET BOUNDED!" : "❌") << "\n";

    return 0;
}
