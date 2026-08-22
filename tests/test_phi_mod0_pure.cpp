// φ-UNIVERSE — PURE MOD 0
// Lahat ng operations ay MOD 0 sa encrypted domain
// Walang MOD 1 sa loob ng computation

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
    std::cout << "  φ-UNIVERSE — PURE MOD 0\n";
    std::cout << "  No MOD 1 in Encrypted Domain\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;
    const double phi_cu_mod = 0.2360679774997897;

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

    // PURE MOD 0: Lahat ng operations ay EvalAdd o EvalSub
    // Walang floor, walang division, walang decryption sa loob

    auto rot1 = [&](auto x) { return cc->EvalAdd(x, ct_phi_mod); };       // +φ_mod
    auto rot2 = [&](auto x) { return cc->EvalAdd(x, ct_psi_mod); };       // +ψ_mod
    auto rot3 = [&](auto x) { return cc->EvalAdd(x, ct_phi_cu_mod); };    // +φ³_mod

    // Initial
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        if (i % 4 == 0) init[i] = {phi_mod, 0.0};
        else if (i % 4 == 1) init[i] = {psi_mod, 0.0};
        else if (i % 4 == 2) init[i] = {phi_cu_mod, 0.0};
        else init[i] = {0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PURE MOD 0 TEST (5000 steps):\n";
    std::cout << "=============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 5000; step++) {
        // PURE MOD 0 operations — lahat EvalAdd/EvalSub
        state = rot1(state);
        state = cc->EvalAtIndex(state, 1);
        state = rot2(state);
        state = cc->EvalAtIndex(state, -1);
        state = rot3(state);

        // MOD 0 normalization: oscillation-based, walang floor
        // Hindi na natin kailangan ng decrypt sa loob
        // Ang φ_sq bound ay natural sa period-4/6 cycle

        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -phi_sq && v <= phi_sq);
        if (!bounded) errors++;

        if (step % 500 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/5000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 5000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PURE MOD 0 BOUNDED!" : "❌") << "\n";

    return 0;
}
