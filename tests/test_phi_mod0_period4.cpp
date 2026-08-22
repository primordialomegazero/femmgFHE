// φ-UNIVERSE — PURE MOD 0 + PERIOD-4
// Period-4 cycle bilang natural na modulo
// Walang decryption sa loob ng computation

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
    std::cout << "  PURE MOD 0 + PERIOD-4\n";
    std::cout << "  Natural Modulo in Encrypted Domain\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
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

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct_phi_sq = make_uniform(phi_sq);
    auto ct_two_phi_sq = make_uniform(two_phi_sq);
    auto ct_three_phi_sq = make_uniform(three_phi_sq);

    // Initial
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 4 == 0) ? phi_sq : (i % 4 == 1) ? 0.0 : (i % 4 == 2) ? two_phi_sq : -phi_sq, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PERIOD-4 MOD 0 (10000 steps):\n";
    std::cout << "=============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 10000; step++) {
        // Period-4 cycle: +φ², +φ², -3φ², +φ²
        if (step % 4 == 2) {
            state = cc->EvalSub(state, ct_three_phi_sq);
        } else {
            state = cc->EvalAdd(state, ct_phi_sq);
        }

        // Bounded check
        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -phi_sq && v <= 3 * phi_sq);
        if (!bounded) errors++;

        if (step % 1000 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/10000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 10000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PERIOD-4 MOD 0 BOUNDED!" : "❌") << "\n";

    return 0;
}
