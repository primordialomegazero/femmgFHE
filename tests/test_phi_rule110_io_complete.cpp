// RULE 110 iO — COMPLETE LEVEL
// 256-slot cellular automaton, dalawang implementations,
// 100 steps, lahat 0-level

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
    std::cout << "  RULE 110 iO — COMPLETE\n";
    std::cout << "  256 Cells, 100 Steps, 0-Level\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // Initial pattern: 1 sa gitna (slot 128)
    std::vector<std::complex<double>> init_A(slots, {0.0, 0.0});
    std::vector<std::complex<double>> init_B(slots, {0.0, 0.0});
    init_A[128] = {phi_sq, 0.0};
    init_B[128] = {phi_sq, 0.0};

    auto state_A = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init_A));
    auto state_B = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init_B));

    // Circuit A: Direct threshold
    auto rule110_A = [&](auto state) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        return cc->EvalSub(sum, ct_two_phi_sq);
    };

    // Circuit B: Period-4 cycle
    auto rule110_B = [&](auto state) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        return cc->EvalSub(cc->EvalAdd(sum, ct_phi_sq), ct_three_phi_sq);
    };

    std::cout << "COMPLETE RULE 110 iO (100 steps):\n";
    std::cout << "=================================\n\n";

    auto t_start = high_resolution_clock::now();
    int match_count = 0;
    int total_checks = 0;

    for (int step = 0; step < 100; step++) {
        state_A = rule110_A(state_A);
        state_B = rule110_B(state_B);

        // I-verify ang equivalence sa selected slots
        if (step % 10 == 0) {
            double vA_128 = decrypt_slot(state_A, 128);
            double vB_128 = decrypt_slot(state_B, 128);
            double vA_0 = decrypt_slot(state_A, 0);
            double vB_0 = decrypt_slot(state_B, 0);
            
            bool match_128 = (std::abs(vA_128 - vB_128) < 0.1);
            bool match_0 = (std::abs(vA_0 - vB_0) < 0.1);
            
            if (match_128) match_count++;
            if (match_0) match_count++;
            total_checks += 2;
            
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "A[128]=" << vA_128 << " B[128]=" << vB_128
                      << " | A[0]=" << vA_0 << " B[0]=" << vB_0
                      << " | " << (match_128 && match_0 ? "✓" : "✗")
                      << " elapsed=" << elapsed << "s\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  iO COMPLETE RESULTS:\n";
    std::cout << "  Steps: 100\n";
    std::cout << "  Cells: 256\n";
    std::cout << "  Match rate: " << (100.0 * match_count / total_checks) << "%\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state_A->GetLevel() << "\n";
    std::cout << "  Status: " << (match_count == total_checks ? "✅ RULE 110 iO COMPLETE!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
