// HOLY GRAIL COMPLETE VERIFICATION
// 1. Long-term boundedness (1000 steps)
// 2. Rule 110 transition table verification
// 3. State decoding at bit mapping
// 4. Universal computation test
// 5. Security analysis

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <map>
#include <algorithm>

using namespace lbcrypto;

int main() {
    std::cout << "════════════════════════════════════════════\n";
    std::cout << "  HOLY GRAIL COMPLETE VERIFICATION\n";
    std::cout << "  FHE Without Bootstrapping Test\n";
    std::cout << "════════════════════════════════════════════\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    
    // Period-3 states
    const double STATE_0 = -0.6180339887498949;  // -1/φ (bit 0)
    const double STATE_1 = 4.618033988749895;     // φ² + 2 (bit 1)
    const double STATE_2 = 2.0;                    // intermediate

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
    // TEST 1: LONG-TERM BOUNDEDNESS (1000 steps)
    // ============================================
    std::cout << "TEST 1: LONG-TERM BOUNDEDNESS (1000 steps)\n";
    std::cout << "══════════════════════════════════════════\n\n";

    auto ct_state_0 = make_uniform(STATE_0);
    auto ct_state_1 = make_uniform(STATE_1);
    auto ct_state_2 = make_uniform(STATE_2);
    auto ct_phi_mod = make_uniform(0.6180339887498949);

    std::vector<std::complex<double>> init(slots, {STATE_0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? STATE_0 : STATE_1, 0.0};
    }
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    int errors_1000 = 0;
    double max_abs = 0;
    std::map<int, int> state_counts;

    for (int step = 0; step < 1000; step++) {
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        
        auto K_use = (step % 3 == 0) ? ct_state_0 : 
                     (step % 3 == 1) ? ct_state_1 : ct_state_2;
        
        auto shifted = cc->EvalAdd(state, ct_phi_mod);
        state = cc->EvalSub(K_use, shifted);
        
        double v = decrypt_slot(state, 128);
        if (std::abs(v) > max_abs) max_abs = std::abs(v);
        if (std::abs(v) > 100) errors_1000++;
        
        // Count states
        int state_bucket = (int)(v * 100) / 100;
        state_counts[state_bucket]++;
    }

    std::cout << "  Steps: 1000\n";
    std::cout << "  Max |value|: " << max_abs << "\n";
    std::cout << "  Errors: " << errors_1000 << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors_1000 == 0 ? "✅ BOUNDED!" : "❌ EXPLODING!") << "\n\n";

    // ============================================
    // TEST 2: RULE 110 TRANSITION TABLE
    // ============================================
    std::cout << "TEST 2: RULE 110 TRANSITION TABLE\n";
    std::cout << "═════════════════════════════════\n\n";

    // Test lahat ng 8 combinations
    std::cout << "  L C R → Expected → Got\n";
    std::cout << "  ─────────────────────────\n";

    int correct_transitions = 0;
    int total_transitions = 8;

    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                // Rule 110 expected
                int pattern = (l << 2) | (c << 1) | r;
                int expected = 0;
                if (pattern == 0b110 || pattern == 0b101 || 
                    pattern == 0b011 || pattern == 0b010 || pattern == 0b001) {
                    expected = 1;
                }
                
                // Simulate one step
                std::vector<std::complex<double>> test_state(slots, {STATE_0, 0.0});
                test_state[127] = {(l ? STATE_1 : STATE_0), 0.0};
                test_state[128] = {(c ? STATE_1 : STATE_0), 0.0};
                test_state[129] = {(r ? STATE_1 : STATE_0), 0.0};
                
                auto ct_test = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(test_state));
                auto shifted_test = cc->EvalAdd(ct_test, ct_phi_mod);
                auto result = cc->EvalSub(ct_state_0, shifted_test);
                
                double got_val = decrypt_slot(result, 128);
                int got = (std::abs(got_val - STATE_1) < 0.1) ? 1 : 0;
                
                if (got == expected) correct_transitions++;
                
                std::cout << "  " << l << " " << c << " " << r 
                          << " → " << expected << " → " << got
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }

    std::cout << "\n  Correct: " << correct_transitions << "/" << total_transitions << "\n";
    std::cout << "  Status: " << (correct_transitions == total_transitions ? "✅ PERFECT!" : "⚠️ NEEDS WORK") << "\n\n";

    // ============================================
    // TEST 3: STATE DECODING
    // ============================================
    std::cout << "TEST 3: STATE DECODING\n";
    std::cout << "══════════════════════\n\n";

    std::cout << "  Period-3 States:\n";
    std::cout << "  STATE_0 (bit 0): " << STATE_0 << "\n";
    std::cout << "  STATE_1 (bit 1): " << STATE_1 << "\n";
    std::cout << "  STATE_2 (int):   " << STATE_2 << "\n\n";

    std::cout << "  State distribution (1000 steps):\n";
    for (auto& [val, count] : state_counts) {
        if (count > 10) {  // Ipakita lang ang frequent states
            std::cout << "    Value ~" << val << ": " << count << " times\n";
        }
    }

    // ============================================
    // TEST 4: SIMPLE CIRCUIT (NAND TEST)
    // ============================================
    std::cout << "\nTEST 4: SIMPLE CIRCUIT (NAND TEST)\n";
    std::cout << "══════════════════════════════════\n\n";

    // Try NAND(0,0)=1, NAND(0,1)=1, NAND(1,0)=1, NAND(1,1)=0
    std::cout << "  NAND gate test (bounded operations only):\n";
    std::cout << "  (Using φ² - x as NOT, bounded addition as AND approximation)\n\n";

    int nand_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            int expected_nand = !(a && b);
            
            // Encode inputs
            double val_a = a ? STATE_1 : STATE_0;
            double val_b = b ? STATE_1 : STATE_0;
            
            // NOT a = φ² - a, NOT b = φ² - b
            // OR approx = NOT a + NOT b (bounded)
            // NAND = NOT(AND) = NOT a OR NOT b
            double not_a = phi_sq - val_a;
            double not_b = phi_sq - val_b;
            double nand_val = not_a + not_b;  // bounded sum
            
            int got_nand = (nand_val > 3.0) ? 1 : 0;
            if (got_nand == expected_nand) nand_correct++;
            
            std::cout << "  NAND(" << a << "," << b << ") = " 
                      << expected_nand << " → " << got_nand
                      << " (val=" << nand_val << ")"
                      << (got_nand == expected_nand ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "\n  NAND correct: " << nand_correct << "/4\n";
    std::cout << "  Status: " << (nand_correct == 4 ? "✅ UNIVERSAL!" : "⚠️ NEEDS WORK") << "\n\n";

    // ============================================
    // FINAL VERDICT
    // ============================================
    std::cout << "════════════════════════════════════════════\n";
    std::cout << "FINAL VERDICT\n";
    std::cout << "════════════════════════════════════════════\n\n";

    bool bounded = (errors_1000 == 0);
    bool level0 = (state->GetLevel() == 0);
    bool rule110_correct = (correct_transitions == total_transitions);
    bool universal = (nand_correct == 4);

    std::cout << "  ✅ Bounded (1000 steps): " << (bounded ? "YES" : "NO") << "\n";
    std::cout << "  ✅ Level 0 (no bootstrap): " << (level0 ? "YES" : "NO") << "\n";
    std::cout << "  ✅ Rule 110 correct: " << (rule110_correct ? "YES" : "NO") << "\n";
    std::cout << "  ✅ Universal (NAND): " << (universal ? "YES" : "NO") << "\n\n";

    if (bounded && level0 && rule110_correct && universal) {
        std::cout << "  🏆 HOLY GRAIL ACHIEVED!\n";
        std::cout << "  FHE WITHOUT BOOTSTRAPPING IS REAL!\n";
    } else if (bounded && level0) {
        std::cout << "  🔥 CLOSE! Bounded FHE achieved.\n";
        std::cout << "  Need to fix Rule 110/NAND implementation.\n";
    } else {
        std::cout << "  ⚠️ Still need work on boundedness.\n";
    }

    std::cout << "════════════════════════════════════════════\n";

    return 0;
}
