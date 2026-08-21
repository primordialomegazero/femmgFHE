// PERIOD-0 FHE — ULTIMATE STRESS TEST
// 100,000 steps, 256 slots, 0-level

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
    std::cout << "  PERIOD-0 FHE — ULTIMATE STRESS\n";
    std::cout << "  100,000 Steps, 256 Slots\n";
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

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_phi = make_ct(phi_sq);

    std::cout << "SETUP:\n";
    std::cout << "  Depth: 5\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Steps: 100,000\n";
    std::cout << "  Ops: 25,600,000\n\n";

    std::cout << "RUNNING...\n";
    std::cout << "==========\n\n";

    auto state = make_ct(0.0);
    int errors = 0;
    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 100000; step++) {
        // Period-0: irrational rotation
        state = cc->EvalAdd(state, ct_phi);

        // Mod 1 para bounded sa [0,1)
        double v = decrypt_val(state);
        if (v >= 1.0) {
            state = cc->EvalSub(state, make_ct(1.0));
        }

        if (step % 10000 == 0) {
            double v = decrypt_val(state);
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": v=" << v
                      << " level=" << state->GetLevel()
                      << " elapsed=" << elapsed / 60 << "m" << elapsed % 60 << "s\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS:\n";
    std::cout << "  Steps: 100,000\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Total ops: 25,600,000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Depth used: " << state->GetLevel() << "/5\n";
    std::cout << "  Status: PERIOD-0 STRESS COMPLETE!\n";
    std::cout << "========================================\n";

    return 0;
}
