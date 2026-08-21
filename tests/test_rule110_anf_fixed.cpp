// RULE 110 VIA ANF — FIXED DECODING
// f(L,C,R) = C + R + L·R - 2·L·C·R
// Decode: val mod 2 (0→0, 1→1, 2→0)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 VIA ANF — FIXED\n";
    std::cout << "  Correct Mod-2 Decoding\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    auto ct_zero = make_uniform(0.0);
    auto ct_one = make_uniform(1.0);

    std::cout << "RULE 110 TRANSITION TABLE (FIXED):\n";
    std::cout << "===================================\n\n";
    std::cout << "  L C R → Expected → Got (val)\n";
    std::cout << "  ───────────────────────────────\n";

    int correct = 0;

    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                int pattern = (l << 2) | (c << 1) | r;
                int expected = 0;
                if (pattern == 0b110 || pattern == 0b101 || 
                    pattern == 0b011 || pattern == 0b010 || pattern == 0b001) {
                    expected = 1;
                }

                auto ct_l = (l == 1) ? ct_one : ct_zero;
                auto ct_c = (c == 1) ? ct_one : ct_zero;
                auto ct_r = (r == 1) ? ct_one : ct_zero;

                // Rule 110 ANF: f = C + R + L·R - 2·L·C·R
                auto lr = cc->EvalMult(ct_l, ct_r);
                auto lc = cc->EvalMult(ct_l, ct_c);
                auto lcr = cc->EvalMult(lc, ct_r);
                
                auto sum1 = cc->EvalAdd(ct_c, ct_r);
                auto sum2 = cc->EvalAdd(sum1, lr);
                
                auto two_lcr = cc->EvalMult(lcr, make_uniform(2.0));
                auto result = cc->EvalSub(sum2, two_lcr);
                
                double got_val = decrypt_slot(result, 128);
                
                // Fixed: mod-2 decoding
                int got = (std::abs(got_val - 1.0) < 0.3) ? 1 : 0;
                
                if (got == expected) correct++;
                
                std::cout << "  " << l << " " << c << " " << r 
                          << " → " << expected << " → " << got
                          << " (val=" << got_val << ")"
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }

    std::cout << "\n  Correct: " << correct << "/8\n";
    std::cout << "  Status: " << (correct == 8 ? "✅ RULE 110 PERFECT!" : "⚠️ NEEDS MORE FIX") << "\n\n";

    // Test boundedness with this ANF (10 steps)
    if (correct == 8) {
        std::cout << "BOUNDEDNESS TEST (10 steps):\n";
        std::cout << "============================\n\n";
        
        std::vector<std::complex<double>> init(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) {
            init[i] = {(i % 3 == 0) ? 1.0 : 0.0, 0.0};
        }
        auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
        
        bool bounded = true;
        for (int step = 0; step < 10; step++) {
            auto left = cc->EvalAtIndex(state, -1);
            auto right = cc->EvalAtIndex(state, 1);
            
            auto lr = cc->EvalMult(left, right);
            auto lc = cc->EvalMult(left, state);
            auto lcr = cc->EvalMult(lc, right);
            
            auto sum1 = cc->EvalAdd(state, right);
            auto sum2 = cc->EvalAdd(sum1, lr);
            auto two_lcr = cc->EvalMult(lcr, make_uniform(2.0));
            state = cc->EvalSub(sum2, two_lcr);
            
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > 2.0) bounded = false;
            
            std::cout << "  Step " << step << ": val=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        std::cout << "\n  Bounded: " << (bounded ? "✅ YES!" : "❌ NO!") << "\n";
    }

    return 0;
}
