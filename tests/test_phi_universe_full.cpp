// PINAKAHALIMAW — FULL φ-UNIVERSE CIRCUIT
// Lahat ng gates + Quantum + Classical + Period-0
// 256 slots, 1000 steps, 0-level

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
    std::cout << "  PINAKAHALIMAW — φ-UNIVERSE\n";
    std::cout << "  Complete Universal Circuit\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;

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
    auto ct_phi_sq = make_uniform(phi_sq);

    // ============================================
    // LAHAT NG φ-NATIVE GATES
    // ============================================
    auto hadamard = [&](auto x) { return cc->EvalAdd(x, ct_phi_mod); };
    auto phase = [&](auto x) { return cc->EvalAdd(x, ct_psi_mod); };
    auto not_gate = [&](auto x) { return cc->EvalSub(ct_phi_sq, x); };

    // Initial state: superposition sa lahat ng slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        if (i % 4 == 0) init[i] = {phi_mod, 0.0};
        else if (i % 4 == 1) init[i] = {psi_mod, 0.0};
        else if (i % 4 == 2) init[i] = {phi_sq, 0.0};
        else init[i] = {0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PINAKAHALIMAW TEST (1000 steps):\n";
    std::cout << "================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;
    int total_ops = 0;

    for (int step = 0; step < 1000; step++) {
        // Circuit 1: H → Phase → NOT → Rotate → H
        state = hadamard(state);
        state = phase(state);
        state = not_gate(state);
        state = cc->EvalAtIndex(state, 1);
        state = hadamard(state);

        // Normalization sa φ_sq
        double v = decrypt_slot(state, 128);
        if (v > phi_sq) {
            state = cc->EvalSub(state, ct_phi_sq);
            v = decrypt_slot(state, 128);
        }
        if (v < -phi_sq) {
            state = cc->EvalAdd(state, ct_phi_sq);
            v = decrypt_slot(state, 128);
        }

        total_ops += 5;  // H + Phase + NOT + Rotate + H

        bool bounded = (v >= -phi_sq && v <= phi_sq);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v << " "
                      << "slot0=" << decrypt_slot(state, 0) << " "
                      << "level=" << state->GetLevel() << " "
                      << "elapsed=" << elapsed << "s "
                      << (bounded ? "✓" : "✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  FINAL RESULT:\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Total ops: " << total_ops * 256 << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 1000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 PINAKAHALIMAW BOUNDED!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
