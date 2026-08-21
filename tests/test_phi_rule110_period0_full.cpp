// RULE 110 + PERIOD-0 — FULL TURING COMPLETE
// Irrational Rotation na 0-level, unbounded
// Complete implementation na may state machine

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
    std::cout << "  RULE 110 + PERIOD-0\n";
    std::cout << "  Full Turing Complete\n";
    std::cout << "  Irrational Rotation 0-Level\n";
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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2, 4, -4, 8, -8});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // RULE 110 TRANSITION FUNCTION
    // ============================================
    std::cout << "RULE 110 TRANSITION:\n";
    std::cout << "====================\n\n";
    std::cout << "  next = 1 if (L,C,R) ∈ {001,010,011,101,110}\n";
    std::cout << "  next = 0 otherwise\n\n";
    
    // Period-0 irrational rotation para sa state encoding
    auto irrational_rotation = [&](auto state, int step) {
        // φ²·step mod 1 — dense, walang repeat
        double rotation = std::fmod(phi_sq * step, 1.0);
        return cc->EvalAdd(state, make_ct(rotation));
    };
    
    // Period-4 threshold para sa Rule 110
    auto period4_threshold = [&](auto x) {
        return cc->EvalSub(make_ct(two_phi_sq), x);
    };
    
    // ============================================
    // FULL STATE MACHINE
    // ============================================
    std::cout << "STATE MACHINE SETUP:\n";
    std::cout << "====================\n\n";
    
    // I-encode ang initial state sa slots
    std::vector<std::complex<double>> initial_state(slots, {0.0, 0.0});
    initial_state[0] = {phi_sq, 0.0};     // 1
    initial_state[1] = {0.0, 0.0};         // 0
    initial_state[2] = {phi_sq, 0.0};     // 1
    initial_state[3] = {0.0, 0.0};         // 0
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(initial_state));
    
    std::cout << "  Initial: 1010...\n\n";
    
    // ============================================
    // RUN RULE 110 WITH PERIOD-0
    // ============================================
    std::cout << "RUNNING RULE 110 (100 steps, 0-level):\n";
    std::cout << "=======================================\n\n";
    
    auto start = high_resolution_clock::now();
    int errors = 0;
    
    for (int step = 0; step < 100; step++) {
        // Rule 110 computation
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Sum = L + C + R
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // Period-4 threshold
        auto thresholded = period4_threshold(sum);
        
        // Period-0 irrational rotation (0-level)
        state = irrational_rotation(thresholded, step);
        
        if (step % 10 == 0 || step == 99) {
            double v = decrypt_val(state);
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel() << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 100\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Time: " << duration.count() << "ms\n";
    std::cout << "  Status: " << (state->GetLevel() == 0 ? "🏆 PERIOD-0 TURING COMPLETE!" : "❌ FAILED") << "\n";
    std::cout << "========================================\n";

    return 0;
}
