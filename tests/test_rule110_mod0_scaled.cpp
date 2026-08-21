// RULE 110 + MOD 0 — SCALED NEIGHBOR
// I-scale ang sum para hindi lumaki

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + MOD 0 SCALED\n";
    std::cout << "  Neighbor Scaling\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double K = phi_sq;
    const double SCALE = 1.0 / 3.0;  // Para sa average ng 3 neighbors

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
    std::vector<std::complex<double>> K_vec(slots, {K, 0.0});
    std::vector<std::complex<double>> scale_vec(slots, {SCALE, 0.0});
    
    auto ct_K = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(K_vec));
    auto ct_scale = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(scale_vec));

    // Initial state: 0 at K/2 alternating
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? K / 2.0 : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 SCALED (100 steps):\n";
    std::cout << "===========================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);

        // I-scale ang sum (equivalent sa average)
        auto scaled_sum = cc->EvalMult(sum, ct_scale);

        // MOD 0: K - scaled_sum
        state = cc->EvalSub(ct_K, scaled_sum);

        // Check
        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -K && v <= K);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            double v128 = decrypt_slot(state, 128);
            double v0 = decrypt_slot(state, 0);
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v128
                      << " slot0=" << v0
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ SCALED BOUNDED!" : "❌") << "\n";

    return 0;
}
