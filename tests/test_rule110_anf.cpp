// RULE 110 VIA ALGEBRAIC NORMAL FORM
// f(L,C,R) = C ⊕ R ⊕ (L·R) ⊕ (L·C·R)
// Encoding: bit 0 = 0, bit 1 = 1 (plain values)
// Sa CKKS: approximation ng XOR at AND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 VIA ANF\n";
    std::cout << "  Bounded Multiplication Test\n";
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

    // Encode bits as 0 and 1 (simple encoding)
    auto ct_zero = make_uniform(0.0);
    auto ct_one = make_uniform(1.0);

    // Test all 8 Rule 110 combinations
    std::cout << "RULE 110 TRANSITION TABLE (ANF):\n";
    std::cout << "=================================\n\n";
    std::cout << "  L C R → Expected → Got\n";
    std::cout << "  ─────────────────────────\n";

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

                // Create ciphertext for L, C, R
                auto ct_l = (l == 1) ? ct_one : ct_zero;
                auto ct_c = (c == 1) ? ct_one : ct_zero;
                auto ct_r = (r == 1) ? ct_one : ct_zero;

                // Rule 110 ANF: f = C + R + L·R + L·C·R (mod 2)
                // Sa real numbers: f = C + R + L·R - 2·L·C·R
                
                // L·R
                auto lr = cc->EvalMult(ct_l, ct_r);
                
                // L·C·R
                auto lc = cc->EvalMult(ct_l, ct_c);
                auto lcr = cc->EvalMult(lc, ct_r);
                
                // C + R
                auto sum1 = cc->EvalAdd(ct_c, ct_r);
                
                // C + R + L·R
                auto sum2 = cc->EvalAdd(sum1, lr);
                
                // 2·L·C·R
                auto two_lcr = cc->EvalMult(lcr, make_uniform(2.0));
                
                // f = C + R + L·R - 2·L·C·R
                auto result = cc->EvalSub(sum2, two_lcr);
                
                double got_val = decrypt_slot(result, 128);
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
    std::cout << "  Status: " << (correct == 8 ? "✅ RULE 110 WORKS!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
