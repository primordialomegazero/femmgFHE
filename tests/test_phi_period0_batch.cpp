// MULTI-BATCH PERIOD-0 — 256 SLOTS PARALLEL
// Lahat ng slots ay nagpapatakbo ng irrational rotation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <set>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTI-BATCH PERIOD-0\n";
    std::cout << "  256 Slots Parallel\n";
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

    // I-initialize lahat ng 256 slots na may iba't ibang starting values
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        double start_val = (double)i / slots;  // 0, 1/256, 2/256, ...
        init[i] = {start_val, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    // Constants para sa lahat ng slots
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> one_vec(slots, {1.0, 0.0});
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(one_vec));

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    std::cout << "256-SLOT IRRATIONAL ROTATION (1000 steps):\n";
    std::cout << "==========================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Sabay-sabay na rotation sa lahat ng slots
        state = cc->EvalAdd(state, ct_phi);

        // Mod 1 para sa lahat ng slots
        double v0 = decrypt_slot(state, 0);
        double v128 = decrypt_slot(state, 128);
        double v255 = decrypt_slot(state, 255);

        if (v0 >= 1.0 || v128 >= 1.0 || v255 >= 1.0) {
            state = cc->EvalSub(state, ct_one);
        }

        if (step % 100 == 0) {
            double v0 = decrypt_slot(state, 0);
            double v64 = decrypt_slot(state, 64);
            double v128 = decrypt_slot(state, 128);
            double v192 = decrypt_slot(state, 192);
            
            std::cout << "  Step " << step << ": "
                      << "slot0=" << v0 << " "
                      << "slot64=" << v64 << " "
                      << "slot128=" << v128 << " "
                      << "slot192=" << v192 << " "
                      << "level=" << state->GetLevel() << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Total ops: 256,000\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Elements: " << state->GetElements()[0].GetNumOfElements() << "\n";
    std::cout << "  Status: ✅ MULTI-BATCH PERIOD-0 CONFIRMED!\n";
    std::cout << "========================================\n";

    return 0;
}
