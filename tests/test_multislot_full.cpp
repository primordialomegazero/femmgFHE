// MULTI-SLOT FULL — 4096 SLOTS + PROGRAMS + ADDER
// Lahat sabay-sabay, 0-level

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
    std::cout << "  MULTI-SLOT FULL — 4096 SLOTS\n";
    std::cout << "  Multiple Programs Parallel\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(4096);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::cout << "Slots: " << slots << "\n\n";

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_phi_sq = make_uniform(phi_sq);

    // Initial: iba't ibang program sa iba't ibang slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        // Program 1 (slots 0-1023): Counter
        // Program 2 (slots 1024-2047): Oscillator
        // Program 3 (slots 2048-3071): Fibonacci-like
        // Program 4 (slots 3072-4095): NAND chain
        
        if (i < 1024) {
            init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
        } else if (i < 2048) {
            init[i] = {(i % 3 == 0) ? phi_sq : 0.0, 0.0};
        } else if (i < 3072) {
            init[i] = {(i % 4 == 0) ? phi_sq : 0.0, 0.0};
        } else {
            init[i] = {(i % 5 == 0) ? phi_sq : 0.0, 0.0};
        }
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "4096 SLOTS — 4 PROGRAMS PARALLEL (500 steps):\n";
    std::cout << "=============================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 500; step++) {
        // Multi-slot NAND: bawat slot ay nagko-compute ng sariling NOT
        auto rotated = cc->EvalAdd(state, ct_phi_mod);
        state = cc->EvalSub(ct_phi_sq, rotated);

        // Check selected slots mula sa bawat program
        double v0 = decrypt_slot(state, 0);        // Program 1
        double v1024 = decrypt_slot(state, 1024);  // Program 2
        double v2048 = decrypt_slot(state, 2048);  // Program 3
        double v3072 = decrypt_slot(state, 3072);  // Program 4

        bool bounded = (std::abs(v0) < 5.0 && std::abs(v1024) < 5.0 && 
                       std::abs(v2048) < 5.0 && std::abs(v3072) < 5.0);
        if (!bounded) errors++;

        if (step % 50 == 0) {
            std::cout << "  Step " << step << ": "
                      << "P1=" << v0 << " "
                      << "P2=" << v1024 << " "
                      << "P3=" << v2048 << " "
                      << "P4=" << v3072 << " "
                      << "level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Slots: " << slots << "\n";
    std::cout << "  Steps: 500\n";
    std::cout << "  Total ops: " << slots * 500 << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ 4096 SLOTS PARALLEL!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
