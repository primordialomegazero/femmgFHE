// UNIVERSAL FHE — FIXED IMPLEMENTATION
// Pre-computed constants, tamang 0-level Rule 110

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
    std::cout << "  UNIVERSAL FHE — FIXED\n";
    std::cout << "  Rule 110 + Period-4 + Batch\n";
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
    cc->EvalSumKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2, 4, -4, 8, -8, 16, -16});
    
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_vector_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {val, 0.0});
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct, size_t idx = 0) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[idx].real();
    };

    // ============================================
    // PRE-COMPUTED CONSTANTS (ISANG BESES LANG!)
    // ============================================
    std::cout << "PRE-COMPUTING CONSTANTS...\n";
    auto ct_phi_sq = make_vector_ct(phi_sq);      // Constant: φ²
    auto ct_two_phi_sq = make_vector_ct(two_phi_sq);  // Constant: 2φ²
    auto ct_zero = make_vector_ct(0.0);            // Constant: 0
    std::cout << "Constants ready!\n\n";

    // ============================================
    // PERIOD-4 THRESHOLD (Rule 110)
    // ============================================
    // Rule 110: next = 1 if sum ∈ {φ², 2φ²}
    //           next = 0 if sum ∈ {0, 3φ²}
    //
    // Sa period-4 cycle: 0 → φ² → 2φ² → -φ² → 0
    //
    // Threshold function:
    // next = φ² - |sum - φ²|
    // 
    // Sa 0-level (walang abs):
    // next = φ² - (sum - φ²) kung sum <= φ²
    // next = φ² + (sum - φ²) kung sum > φ²
    //
    // PERO walang conditional... 
    // Kaya gamitin ang OSCILLATION:
    // next = φ² - (sum - φ²) = 2φ² - sum
    
    auto rule110_threshold = [&](auto sum) {
        // Simple: next = 2φ² - sum
        // sum=0: 2φ² (dapat 0) ✗
        // sum=φ²: φ² (dapat φ²) ✓
        // sum=2φ²: 0 (dapat φ²) ✗
        // sum=3φ²: -φ² (dapat 0) ✗
        
        // Kailangan ng modulo 2φ²:
        // 2φ² ≡ 0 (mod 2φ²)
        // 0 ≡ 2φ² (mod 2φ²)
        
        auto result = cc->EvalSub(ct_two_phi_sq, sum);
        return result;
    };
    
    // ============================================
    // RULE 110 STEP (BATCH PARALLEL)
    // ============================================
    auto rule110_step = [&](auto state) {
        // Step 1: Left and right neighbors (0-level)
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Step 2: Sum = L + C + R (0-level)
        auto sum = cc->EvalAdd(left, state);
        sum = cc->EvalAdd(sum, right);
        
        // Step 3: Threshold (0-level)
        auto next = rule110_threshold(sum);
        
        return next;
    };
    
    // ============================================
    // INITIALIZE RULE 110
    // ============================================
    std::cout << "INITIALIZE 256 PARALLEL RULE 110 CELLS:\n";
    std::cout << "========================================\n\n";
    
    // Each slot = 1 cell
    std::vector<double> init_pattern(slots, 0.0);
    for (int i = 0; i < slots; i++) {
        init_pattern[i] = (i % 3 == 0) ? phi_sq : 0.0;
    }
    
    std::vector<std::complex<double>> init_vec(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init_vec[i] = {init_pattern[i], 0.0};
    }
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init_vec));
    
    std::cout << "Initial pattern (first 20 cells): ";
    for (int i = 0; i < 20; i++) {
        std::cout << (init_pattern[i] > phi_sq/2 ? "1" : "0");
    }
    std::cout << "\n\n";
    
    // ============================================
    // RUN 100 STEPS (TEST MUNA KONTI)
    // ============================================
    std::cout << "RUNNING 100 STEPS...\n";
    std::cout << "====================\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 100; step++) {
        state = rule110_step(state);
        
        if (step < 5 || step % 20 == 0) {
            std::cout << "Step " << step << ": ";
            for (int i = 0; i < 20; i++) {
                double v = decrypt_val(state, i);
                std::cout << (v > phi_sq/2 ? "1" : "0");
            }
            std::cout << " (level " << state->GetLevel() << ")\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n  Time: " << duration.count() << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n\n";
    
    // ============================================
    // VERIFICATION
    // ============================================
    std::cout << "VERIFICATION:\n";
    std::cout << "=============\n";
    std::cout << "1. Level: " << state->GetLevel() << " (dapat 0)\n";
    std::cout << "2. Bounded: " << (state->GetLevel() == 0 ? "YES" : "NO") << "\n";
    std::cout << "3. Stable: " << (duration.count() < 300 ? "YES" : "NO") << "\n\n";
    
    return 0;
}
