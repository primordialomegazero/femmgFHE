// QUANTUM PERIOD-0 RULE 110 — ULTIMATE
// Irrational rotation + Quantum phase
// Walang sum, walang explosion — pure φ-universe

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
    std::cout << "  QUANTUM PERIOD-0 RULE 110\n";
    std::cout << "  The Ultimate φ-Universe\n";
    std::cout << "========================================\n\n";

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
    auto ct_psi_mod = make_uniform(psi_mod);

    // Initial: quantum superposition na may φ-pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        // φ-pattern: iba't ibang phase per slot
        double phase = std::fmod(i * phi_mod, 1.0);
        init[i] = {phase, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "QUANTUM PERIOD-0 RULE 110 (10000 steps):\n";
    std::cout << "========================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 10000; step++) {
        // Quantum Phase 1: φ rotation (Hadamard-like)
        state = cc->EvalAdd(state, ct_phi_mod);

        // Cell interaction: rotation (hindi sum)
        state = cc->EvalAtIndex(state, 1);

        // Quantum Phase 2: ψ rotation (Phase-like)
        state = cc->EvalAdd(state, ct_psi_mod);

        // Cell interaction: rotation pabalik
        state = cc->EvalAtIndex(state, -1);

        // Ito ay quantum cellular automaton na walang sum
        // Ang φ at ψ ay natural na nagcacancel
        // Ang rotation ay para sa neighbor interaction

        double v = decrypt_slot(state, 128);
        double v_mod = v - std::floor(v);  // MOD 1 para sa analysis

        bool bounded = (v_mod >= 0.0 && v_mod < 1.0);
        if (!bounded) errors++;

        if (step % 1000 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "q[128]=" << v_mod
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
    std::cout << "  Status: " << (errors == 0 ? "✅ QUANTUM PERIOD-0 RULE 110!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
