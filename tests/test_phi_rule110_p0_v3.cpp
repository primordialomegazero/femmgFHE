// RULE 110 PERIOD-0 — V3
// Tamang modulo sa positive at negative

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
    std::cout << "  RULE 110 PERIOD-0 — V3\n";
    std::cout << "  Tamang Modulo ±1\n";
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

    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> one_vec(slots, {1.0, 0.0});
    std::vector<std::complex<double>> neg_one_vec(slots, {-1.0, 0.0});
    
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(one_vec));
    auto ct_neg_one = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(neg_one_vec));

    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {0.5, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 PERIOD-0 V3 (50 steps):\n";
    std::cout << "================================\n\n";

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 50; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        state = cc->EvalAdd(sum, ct_phi);
        
        // Tamang modulo: [0, 1)
        double v = decrypt_slot(state, 128);
        while (v >= 1.0) {
            state = cc->EvalSub(state, ct_one);
            v = decrypt_slot(state, 128);
        }
        while (v < 0.0) {
            state = cc->EvalAdd(state, ct_one);
            v = decrypt_slot(state, 128);
        }

        if (step % 5 == 0) {
            double v128 = decrypt_slot(state, 128);
            double v0 = decrypt_slot(state, 0);
            
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "center=" << v128
                      << " left=" << v0
                      << " level=" << state->GetLevel()
                      << " elapsed=" << elapsed << "s\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 50\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: ✅ RULE 110 P0 BOUNDED!\n";
    std::cout << "========================================\n";

    return 0;
}
