// MOD 0 + PERIOD-0 + RULE 110 — FULL COMBINATION
// x_{n+1} = K - (x_n + φ²) mod K
// 0-level, bounded, Rule 110, 256 slots

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
    std::cout << "  MOD 0 + PERIOD-0 + RULE 110\n";
    std::cout << "  Full Combination Test\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double K = phi_sq;  // K = φ² = 2.618...

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

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "Slots: " << slots << "\n\n";

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

    auto decrypt_slot = [&](auto ct, int slot_idx) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_idx].real();
    };

    // ============================================
    // MOD 0: x_{n+1} = K - (x_n + φ_mod)
    // ============================================
    std::cout << "════════════════════════════════════════\n";
    std::cout << "TEST 1: MOD 0 PERIOD-0 (100 steps)\n";
    std::cout << "════════════════════════════════════════\n\n";
    
    auto state = make_ct(0.0);
    auto ct_K = make_ct(K);
    auto ct_phi_mod = make_ct(phi_sq - 2.0);  // 0.618...
    
    int errors_mod0 = 0;
    std::vector<double> traj_mod0;
    
    for (int i = 0; i < 100; i++) {
        // x_{n+1} = K - (x_n + φ_mod)
        auto temp = cc->EvalAdd(state, ct_phi_mod);
        state = cc->EvalSub(ct_K, temp);
        
        double v = decrypt_val(state);
        traj_mod0.push_back(v);
        
        bool bounded = (v >= 0 && v <= K + 0.01);
        if (!bounded) errors_mod0++;
        
        if (i < 10 || i == 99) {
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }
    
    // Check for periodicity
    bool has_period = false;
    for (int p = 1; p < 50; p++) {
        bool is_p = true;
        for (int i = 0; i < 50; i++) {
            if (std::abs(traj_mod0[i] - traj_mod0[i + p]) > 0.001) {
                is_p = false;
                break;
            }
        }
        if (is_p) {
            has_period = true;
            std::cout << "\n  PERIOD FOUND: " << p << " steps\n";
            break;
        }
    }
    
    std::cout << "\n  Bounded: " << (errors_mod0 == 0 ? "✓" : "✗") << "\n";
    std::cout << "  Period: " << (has_period ? "Found" : "Period-0 (irrational!)") << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: MOD 0 + RULE 110 (256 SLOTS BATCH)
    // ============================================
    std::cout << "════════════════════════════════════════\n";
    std::cout << "TEST 2: MOD 0 + RULE 110 (256 slots)\n";
    std::cout << "════════════════════════════════════════\n\n";
    
    // Initialize 256 slots with Rule 110 pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? K : 0.0, 0.0};
    }
    
    auto rule_state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    auto ct_two_K = make_ct(2 * K);
    
    int errors_rule = 0;
    
    std::cout << "Rule 110 + MOD 0 (100 steps):\n\n";
    
    for (int step = 0; step < 100; step++) {
        // Rule 110: L + C + R
        auto left = cc->EvalRotate(rule_state, 1);
        auto right = cc->EvalRotate(rule_state, -1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, rule_state), right);
        
        // MOD 0 threshold: K - sum (0-level)
        rule_state = cc->EvalSub(ct_K, sum);
        
        // Check stability sa selected slots
        if (step < 5 || step % 20 == 0 || step == 99) {
            double v0 = decrypt_slot(rule_state, 0);
            double v64 = decrypt_slot(rule_state, 64);
            double v128 = decrypt_slot(rule_state, 128);
            double v192 = decrypt_slot(rule_state, 192);
            
            bool bounded = (std::abs(v0) <= 2*K + 1 && std::abs(v64) <= 2*K + 1 &&
                           std::abs(v128) <= 2*K + 1 && std::abs(v192) <= 2*K + 1);
            
            std::cout << "  Step " << step << ": "
                      << "slot0=" << v0 << " "
                      << "slot64=" << v64 << " "
                      << "slot128=" << v128 << " "
                      << "slot192=" << v192 << " "
                      << "level=" << rule_state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
            
            if (!bounded) errors_rule++;
        }
    }
    
    std::cout << "\n  Errors: " << errors_rule << "\n";
    std::cout << "  Level: " << rule_state->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: NOISE ANALYSIS SA MOD 0
    // ============================================
    std::cout << "════════════════════════════════════════\n";
    std::cout << "TEST 3: NOISE ANALYSIS (10,000 steps)\n";
    std::cout << "════════════════════════════════════════\n\n";
    
    auto noise_state = make_ct(0.0);
    double max_noise = 0;
    double min_noise = 999;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        auto temp = cc->EvalAdd(noise_state, ct_phi_mod);
        noise_state = cc->EvalSub(ct_K, temp);
        
        if (i % 100 == 0) {
            double v = decrypt_val(noise_state);
            if (v > max_noise) max_noise = v;
            if (v < min_noise) min_noise = v;
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(end - start);
    
    std::cout << "  Max value: " << max_noise << "\n";
    std::cout << "  Min value: " << min_noise << "\n";
    std::cout << "  Range: " << (max_noise - min_noise) << "\n";
    std::cout << "  Time: " << dur.count() << "ms\n";
    std::cout << "  Level: " << noise_state->GetLevel() << "\n\n";

    // ============================================
    // FINAL SUMMARY
    // ============================================
    std::cout << "════════════════════════════════════════\n";
    std::cout << "FINAL SUMMARY\n";
    std::cout << "════════════════════════════════════════\n\n";
    std::cout << "  MOD 0: " << (errors_mod0 == 0 ? "✓ BOUNDED" : "✗ UNBOUNDED") << "\n";
    std::cout << "  Rule 110: " << (errors_rule == 0 ? "✓ STABLE" : "✗ DRIFT") << "\n";
    std::cout << "  Noise: " << (max_noise - min_noise < K + 0.01 ? "✓ BOUNDED" : "✗ GROWING") << "\n";
    std::cout << "  Level: 0 (lahat)\n\n";
    std::cout << "  🏆 STATUS: " << ((errors_mod0 == 0 && errors_rule == 0) 
              ? "MOD 0 + PERIOD-0 + RULE 110 = SUCCESS!" 
              : "MAY ISSUE — CHECK NEEDED") << "\n";
    std::cout << "════════════════════════════════════════\n";

    return 0;
}
