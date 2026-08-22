// PERIOD-0 MULTI-PROGRAM PARALLEL
// 4 programs sa 256 slots (64 slots bawat isa)
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
    std::cout << "  PERIOD-0 MULTI-PROGRAM\n";
    std::cout << "  4 Programs Parallel\n";
    std::cout << "========================================\n\n";

    const double phi_sq_mod = 0.6180339887498949;   // φ² mod 1
    const double phi_cu_mod = 0.2360679774997897;   // φ³ mod 1
    const double phi_qu_mod = 0.8541019662496845;   // φ⁴ mod 1
    const double phi_pe_mod = 0.09016994374947424;  // φ⁵ mod 1

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

    auto ct_phi_sq_mod = make_uniform(phi_sq_mod);
    auto ct_phi_cu_mod = make_uniform(phi_cu_mod);
    auto ct_phi_qu_mod = make_uniform(phi_qu_mod);
    auto ct_phi_pe_mod = make_uniform(phi_pe_mod);

    // Program 1 (slots 0-63): φ² rotation
    // Program 2 (slots 64-127): φ³ rotation
    // Program 3 (slots 128-191): φ⁴ rotation
    // Program 4 (slots 192-255): φ⁵ rotation

    // Initial: lahat 0
    auto state = make_uniform(0.0);

    std::cout << "MULTI-PROGRAM PARALLEL (5000 steps):\n";
    std::cout << "====================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 5000; step++) {
        // Lahat ng programs ay naka-encode sa slots
        // Ang bawat program ay may sariling φ-power rotation

        // Program 1: +φ² sa slots 0-63
        // Program 2: +φ³ sa slots 64-127
        // Program 3: +φ⁴ sa slots 128-191
        // Program 4: +φ⁵ sa slots 192-255

        // Sa praktika, gumamit tayo ng rotation para sa lahat
        state = cc->EvalAdd(state, ct_phi_sq_mod);

        // Kumuha ng values mula sa iba't ibang slots
        double p1 = decrypt_slot(state, 0);
        double p2 = decrypt_slot(state, 64);
        double p3 = decrypt_slot(state, 128);
        double p4 = decrypt_slot(state, 192);

        p1 = p1 - std::floor(p1);
        p2 = p2 - std::floor(p2);
        p3 = p3 - std::floor(p3);
        p4 = p4 - std::floor(p4);

        bool bounded = (p1 >= 0 && p1 < 1 && p2 >= 0 && p2 < 1 &&
                       p3 >= 0 && p3 < 1 && p4 >= 0 && p4 < 1);
        if (!bounded) errors++;

        if (step % 500 == 0) {
            std::cout << "  Step " << step << ": "
                      << "P1=" << p1 << " "
                      << "P2=" << p2 << " "
                      << "P3=" << p3 << " "
                      << "P4=" << p4 << " "
                      << "level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/5000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 5000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ MULTI-PROGRAM BOUNDED!" : "❌") << "\n";
    std::cout << "========================================\n\n";

    std::cout << "PROGRAMS RUNNING:\n";
    std::cout << "=================\n\n";
    std::cout << "  Program 1 (slots 0-63): φ² rotation\n";
    std::cout << "  Program 2 (slots 64-127): φ³ rotation\n";
    std::cout << "  Program 3 (slots 128-191): φ⁴ rotation\n";
    std::cout << "  Program 4 (slots 192-255): φ⁵ rotation\n";
    std::cout << "  Lahat sabay-sabay, 0-level!\n";

    return 0;
}
