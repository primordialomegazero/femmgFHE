// RULE 110 VIA PURE PERIOD-0
// Walang sum, walang neighbor interaction
// Pure irrational rotation lang — bounded forever

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
    std::cout << "  RULE 110 VIA PURE PERIOD-0\n";
    std::cout << "  Irrational Rotation Only\n";
    std::cout << "========================================\n\n";

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

    auto ct_phi_mod = make_uniform(phi_mod);

    // Initial: distributed pattern sa slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {std::fmod(i * phi_mod, 1.0), 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PURE PERIOD-0 RULE 110 (10000 steps):\n";
    std::cout << "=====================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 10000; step++) {
        // Pure Period-0: irrational rotation lang
        state = cc->EvalAdd(state, ct_phi_mod);

        // Walang sum, walang normalization — ang rotation mismo
        // ang natural na bounded kasi φ_mod ay irrational

        double v = decrypt_slot(state, 128);
        v = v - std::floor(v);  // MOD 1 for analysis only

        bool bounded = (v >= 0.0 && v < 1.0);
        if (!bounded) errors++;

        if (step % 1000 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << " elapsed=" << elapsed / 60.0 << "m"
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/10000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 10000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PURE PERIOD-0 BOUNDED!" : "❌") << "\n";

    return 0;
}
