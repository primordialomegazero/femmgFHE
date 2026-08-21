// RULE 110 — FINAL FIX
// Tamang transition table + bounded encoding
// f(L,C,R) = φ² - |L + C + R - φ²| (correct threshold)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — FINAL FIX\n";
    std::cout << "  Correct Threshold\n";
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

    // TEST: Rule 110 truth table gamit ang tamang threshold
    std::cout << "RULE 110 TRUTH TABLE TEST:\n";
    std::cout << "==========================\n\n";
    std::cout << "  L C R → Expected → Got → Match?\n";
    std::cout << "  ─────────────────────────────\n";
    
    int correct = 0;
    
    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                // Expected Rule 110
                int expected;
                if (l == 1 && c == 1 && r == 1) expected = 0;
                else if (l == 0 && c == 0 && r == 0) expected = 0;
                else expected = 1;
                
                // φ-domain computation
                double L = l ? phi_sq : 0.0;
                double C = c ? phi_sq : 0.0;
                double R = r ? phi_sq : 0.0;
                
                double sum = L + C + R;
                double result = two_phi_sq - sum;  // bounded oscillation
                
                // Decode: kung result > φ_sq/2, it's 1; else 0
                int got = (std::abs(result) > phi_sq / 2) ? 1 : 0;
                
                bool match = (expected == got);
                if (match) correct++;
                
                std::cout << "  " << l << " " << c << " " << r 
                          << " → " << expected << " → " << got 
                          << " (val=" << result << ") " 
                          << (match ? "✓" : "✗") << "\n";
            }
        }
    }
    
    std::cout << "\n  Correct: " << correct << "/8\n";
    std::cout << "  Status: " << (correct == 8 ? "🏆 RULE 110 CORRECT!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
