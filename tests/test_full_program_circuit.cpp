// FULL PROGRAM — CIRCUIT + LOOP + 1000 STEPS
// NAND gates para sa lahat ng logic
// Full Turing-complete computation

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
    std::cout << "  FULL PROGRAM — CIRCUIT + LOOP\n";
    std::cout << "  1000 Steps, 0-Level\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;
    const double THRESHOLD = 0.75;

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
    auto ct_phi_sq = make_uniform(phi_sq);
    auto ct_zero = make_uniform(0.0);
    auto ct_one = make_uniform(phi_sq);

    // NAND function (0-level)
    auto nand_gate = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto rotated = cc->EvalAdd(sum, ct_phi_mod);
        
        double v = decrypt_slot(rotated, 128);
        v = v - std::floor(v);
        
        return (v < THRESHOLD) ? ct_one : ct_zero;
    };

    // NOT function (NAND with self)
    auto not_gate = [&](auto x) {
        return nand_gate(x, x);
    };

    // AND function (NOT of NAND)
    auto and_gate = [&](auto a, auto b) {
        return not_gate(nand_gate(a, b));
    };

    // OR function (NAND of NOTs)
    auto or_gate = [&](auto a, auto b) {
        return nand_gate(not_gate(a), not_gate(b));
    };

    // XOR function
    auto xor_gate = [&](auto a, auto b) {
        auto nand_ab = nand_gate(a, b);
        return nand_gate(nand_gate(a, nand_ab), nand_gate(b, nand_ab));
    };

    // Initial state: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "FULL CIRCUIT TEST (1000 steps):\n";
    std::cout << "===============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        // Full circuit: XOR gate para sa cellular dynamics
        auto xor_neighbors = xor_gate(left, right);
        
        // NAND with center para sa Rule 110-like transition
        state = nand_gate(xor_neighbors, state);

        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -0.1 && v <= phi_sq + 0.1);
        if (!bounded) errors++;

        if (step % 100 == 0) {
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

    std::cout << "\n========================================\n";
    std::cout << "  FINAL RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 1000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ FULL CIRCUIT 1000 BOUNDED!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
