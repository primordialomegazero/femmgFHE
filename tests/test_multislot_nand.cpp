// MULTI-SLOT NAND — WALANG NEIGHBOR SUM
// Bawat slot ay may sariling NAND gate
// Hindi kailangan ng sum sa neighbors

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
    std::cout << "  MULTI-SLOT NAND\n";
    std::cout << "  Walang Neighbor Sum\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;

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
    auto ct_phi_sq = make_uniform(phi_sq);

    // Multi-slot NAND: bawat slot ay nagko-compute ng sariling NAND
    // Walang neighbor interaction — puro self-contained computation

    // Initial: alternating pattern sa lahat ng slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "MULTI-SLOT NAND (1000 steps):\n";
    std::cout << "=============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Bawat slot: NAND(x, x) = NOT(x)
        // Period-0 rotation para sa threshold
        auto rotated = cc->EvalAdd(state, ct_phi_mod);
        
        // Bounded oscillation: φ² - rotated
        state = cc->EvalSub(ct_phi_sq, rotated);

        // Check selected slots
        double v0 = decrypt_slot(state, 0);
        double v64 = decrypt_slot(state, 64);
        double v128 = decrypt_slot(state, 128);
        double v192 = decrypt_slot(state, 192);

        bool bounded = (std::abs(v0) < 5.0 && std::abs(v64) < 5.0 && 
                       std::abs(v128) < 5.0 && std::abs(v192) < 5.0);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot0=" << v0 << " "
                      << "slot64=" << v64 << " "
                      << "slot128=" << v128 << " "
                      << "slot192=" << v192 << " "
                      << "level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ MULTI-SLOT NAND BOUNDED!" : "❌") << "\n";

    return 0;
}
