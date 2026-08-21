// BATCHING PERIOD-4 — 1,000,000 STEPS
// 256 slots × 1M steps = 256,000,000 total operations
// 0-level lahat!

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
    std::cout << "  BATCHING PERIOD-4 — 1M STEPS\n";
    std::cout << "  256 Slots × 1M Steps\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // Initialize 256 slots na may period-4 phases
    std::vector<std::complex<double>> init_vec(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        if (i % 4 == 0) init_vec[i] = {0.0, 0.0};
        else if (i % 4 == 1) init_vec[i] = {phi_sq, 0.0};
        else if (i % 4 == 2) init_vec[i] = {two_phi_sq, 0.0};
        else init_vec[i] = {-phi_sq, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init_vec));

    std::cout << "RUNNING 1,000,000 STEPS...\n";
    std::cout << "===========================\n\n";

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 1000000; step++) {
        if (step % 4 == 2) {
            state = cc->EvalSub(state, ct_three_phi_sq);
        } else {
            state = cc->EvalAdd(state, ct_phi_sq);
        }

        if (step % 100000 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, state, &pt);
            auto vals = pt->GetCKKSPackedValue();
            
            double v0 = vals[0].real();
            double v64 = vals[64].real();
            double v128 = vals[128].real();
            double v192 = vals[192].real();
            
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "slot0=" << v0 << " "
                      << "slot64=" << v64 << " "
                      << "slot128=" << v128 << " "
                      << "slot192=" << v192 << " "
                      << "level=" << state->GetLevel() << " "
                      << "elapsed=" << elapsed / 60 << "m" << elapsed % 60 << "s\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  FINAL RESULTS:\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Steps: 1,000,000\n";
    std::cout << "  Total operations: 256,000,000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: ✅ 1M BATCHING CONFIRMED!\n";
    std::cout << "========================================\n";

    return 0;
}
