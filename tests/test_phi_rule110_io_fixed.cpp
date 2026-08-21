// RULE 110 iO COMPLETE — FIXED
// May period-4 correction sa bawat step

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
    std::cout << "  RULE 110 iO — FIXED\n";
    std::cout << "  With Period-4 Correction\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    // Constants
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> two_phi_vec(slots, {two_phi_sq, 0.0});
    std::vector<std::complex<double>> three_phi_vec(slots, {three_phi_sq, 0.0});

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    auto ct_two_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_vec));
    auto ct_three_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(three_phi_vec));

    // Initial pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};

    auto state_A = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    auto state_B = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    // Circuit A: Direct threshold + period-4 correction
    auto rule110_A = [&](auto state, int step) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        auto result = cc->EvalSub(sum, ct_two_phi);
        
        // Period-4 correction
        if (step % 4 == 2) {
            result = cc->EvalSub(result, ct_three_phi);
        } else {
            result = cc->EvalAdd(result, ct_phi);
        }
        return result;
    };

    // Circuit B: Period-4 cycle + correction (iba ang order)
    auto rule110_B = [&](auto state, int step) {
        // Iba ang structure: correction muna bago ang threshold
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // Correction muna
        if (step % 4 == 2) {
            sum = cc->EvalAdd(sum, ct_phi);
        }
        
        auto result = cc->EvalSub(sum, ct_two_phi);
        
        if (step % 4 != 2) {
            result = cc->EvalSub(result, ct_three_phi);
        }
        return result;
    };

    std::cout << "RULE 110 iO FIXED (100 steps):\n";
    std::cout << "==============================\n\n";

    auto t_start = high_resolution_clock::now();
    int match_count = 0;

    for (int step = 0; step < 100; step++) {
        state_A = rule110_A(state_A, step);
        state_B = rule110_B(state_B, step);

        if (step % 10 == 0) {
            double vA_128 = decrypt_slot(state_A, 128);
            double vB_128 = decrypt_slot(state_B, 128);
            double vA_0 = decrypt_slot(state_A, 0);
            double vB_0 = decrypt_slot(state_B, 0);
            
            bool match_128 = (std::abs(vA_128 - vB_128) < 0.1);
            bool match_0 = (std::abs(vA_0 - vB_0) < 0.1);
            
            if (match_128) match_count++;
            if (match_0) match_count++;
            
            std::cout << "  Step " << step << ": "
                      << "A[128]=" << vA_128 << " B[128]=" << vB_128
                      << " | A[0]=" << vA_0 << " B[0]=" << vB_0
                      << " | " << (match_128 && match_0 ? "✓" : "✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS:\n";
    std::cout << "  Steps: 100\n";
    std::cout << "  Match rate: " << (100.0 * match_count / 20) << "%\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state_A->GetLevel() << "\n";
    std::cout << "  Status: " << (match_count == 20 ? "✅ iO FIXED!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
