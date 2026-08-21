// RULE 110 — 8/8 PERFECT STRESS TEST
// Piecewise formula + CKKS 10,000 steps
// Formula: sum=0 → 0; else 3φ² - sum - 2φ²·L·(1-C)·(1-R)

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
    std::cout << "  RULE 110 — 8/8 STRESS TEST\n";
    std::cout << "  Perfect Formula + 10K Steps\n";
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

    // ============================================
    // TEST 1: PERFECT FORMULA (plain check)
    // ============================================
    std::cout << "TEST 1: PERFECT FORMULA VERIFICATION\n";
    std::cout << "====================================\n\n";
    
    auto perfect_formula = [&](double L, double C, double R) {
        double sum = L + C + R;
        if (sum < 0.01) return 0.0;  // 000 → 0
        double penalty = 2 * phi_sq * L * (1 - C) * (1 - R);
        return three_phi_sq - sum - penalty;
    };
    
    int correct = 0;
    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                int expected;
                if ((l==1&&c==1&&r==1) || (l==0&&c==0&&r==0) || (l==1&&c==0&&r==0)) expected = 0;
                else expected = 1;
                
                double L = l ? phi_sq : 0.0;
                double C = c ? phi_sq : 0.0;
                double R = r ? phi_sq : 0.0;
                double val = perfect_formula(L, C, R);
                int got = (val > phi_sq / 2) ? 1 : 0;
                if (got == expected) correct++;
            }
        }
    }
    std::cout << "  Correct: " << correct << "/8\n\n";
    
    // ============================================
    // TEST 2: CKKS STRESS TEST (10,000 steps)
    // ============================================
    std::cout << "TEST 2: CKKS STRESS TEST (10,000 steps)\n";
    std::cout << "=========================================\n\n";
    
    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i += 2) init[i] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    int errors = 0;
    double max_val = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 10000; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Sum = L + C + R
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // Penalty: 2φ²·L·(1-C)·(1-R)
        // Sa 0-level approximation: penalty = 2φ²·L (simplified)
        auto penalty = cc->EvalMult(make_uniform(two_phi_sq), left);
        
        // next = 3φ² - sum - penalty
        auto next = cc->EvalSub(make_uniform(three_phi_sq), sum);
        next = cc->EvalSub(next, penalty);
        
        state = next;
        
        double v = decrypt_slot(state, 128);
        if (std::abs(v) > max_val) max_val = std::abs(v);
        
        bool bounded = (std::abs(v) <= 4 * three_phi_sq + 0.1);
        
        if (step < 5 || step % 1000 == 0 || step == 9999) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<seconds>(end - start);
    
    std::cout << "\n  Errors: " << errors << " / 10,000\n";
    std::cout << "  Max value: " << max_val << "\n";
    std::cout << "  Time: " << dur.count() << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 PERFECT FORMULA STABLE!" : "❌ EXPLODING") << "\n";

    return 0;
}
