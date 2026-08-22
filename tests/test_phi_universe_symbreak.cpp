// φ-UNIVERSE — SYMMETRY BREAKING
// Magdagdag ng asymmetric rotation para hindi ma-stuck

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
    std::cout << "  φ-UNIVERSE — SYMMETRY BREAK\n";
    std::cout << "  Walang Fixed Point\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;
    const double phi_cu_mod = 0.2360679774997897;  // φ³ mod 1

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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});

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
    auto ct_psi_mod = make_uniform(psi_mod);
    auto ct_phi_cu_mod = make_uniform(phi_cu_mod);
    auto ct_phi_sq = make_uniform(phi_sq);

    // Asymmetric gates — 3 iba't ibang rotations
    auto rot1 = [&](auto x) { return cc->EvalAdd(x, ct_phi_mod); };
    auto rot2 = [&](auto x) { return cc->EvalAdd(x, ct_psi_mod); };
    auto rot3 = [&](auto x) { return cc->EvalAdd(x, ct_phi_cu_mod); };

    // Initial
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        if (i % 4 == 0) init[i] = {phi_mod, 0.0};
        else if (i % 4 == 1) init[i] = {psi_mod, 0.0};
        else if (i % 4 == 2) init[i] = {phi_cu_mod, 0.0};
        else init[i] = {0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "SYMMETRY BREAK TEST (1000 steps):\n";
    std::cout << "=================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Asymmetric circuit: 3 iba't ibang rotations
        state = rot1(state);
        state = cc->EvalAtIndex(state, 1);
        state = rot2(state);
        state = cc->EvalAtIndex(state, -1);
        state = rot3(state);

        // Normalization
        double v = decrypt_slot(state, 128);
        if (v > phi_sq) {
            state = cc->EvalSub(state, ct_phi_sq);
            v = decrypt_slot(state, 128);
        }
        if (v < -phi_sq) {
            state = cc->EvalAdd(state, ct_phi_sq);
            v = decrypt_slot(state, 128);
        }

        bool bounded = (v >= -phi_sq && v <= phi_sq);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v << " "
                      << "level=" << state->GetLevel() << " "
                      << (bounded ? "✓" : "✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 1000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ SYMMETRY BREAK BOUNDED!" : "❌") << "\n";

    return 0;
}
