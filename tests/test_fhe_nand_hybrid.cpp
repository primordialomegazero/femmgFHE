// FHE NAND — HYBRID P2+P4
// Natural na pag-switch via period-6
// Walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND — HYBRID P2+P4\n";
    std::cout << "  Period-6 Natural Switch\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_MOD = PHI_SQ - 2.0;
    const double TWO_PHI_SQ = 2 * PHI_SQ;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
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

    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_two_phi_sq = make_ct(TWO_PHI_SQ);
    auto ct_phi_mod = make_ct(PHI_MOD);
    auto ct_zero = make_ct(0.0);

    std::cout << "HYBRID NAND TEST:\n";
    std::cout << "=================\n\n";

    // Test: NAND(a,b) gamit ang sum at hybrid correction
    // Key insight: ang sum a+b ay nasa {0, φ², 2φ²}
    // - sum=0 → dapat φ² (NOT ng 0)
    // - sum=φ² → dapat φ² (NOT ng φ²? Hindi...)
    // - sum=2φ² → dapat 0 (NOT ng 2φ²?)
    
    // Mas maganda: gamitin ang sum para pumili ng period
    // sum=0 → P2 (K - x)
    // sum=φ² → P4 (2φ² - x)  
    // sum=2φ² → P4 (2φ² - x)

    int correct = 0;
    Ciphertext<DCRTPoly> result;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? PHI_SQ : 0.0;
            double b_val = b_bit ? PHI_SQ : 0.0;
            double sum_val = a_val + b_val;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // Compute sum
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            
            // Hybrid: NAND = φ² - sum + correction
            // Kung sum=0: correction = φ² (para maging φ²)
            // Kung sum=φ²: correction = φ² (para maging φ²)
            // Kung sum=2φ²: correction = 0 (para maging 0)
            
            // Sa φ-domain: ang correction ay φ² - sum
            auto correction = cc->EvalSub(ct_phi_sq, sum_ab);
            
            // NAND = φ² - sum + correction = 2φ² - 2·sum
            auto neg_sum = cc->EvalSub(ct_zero, sum_ab);
            auto two_neg_sum = cc->EvalAdd(neg_sum, neg_sum);
            result = cc->EvalAdd(ct_two_phi_sq, two_neg_sum);
            
            // Final: 2φ² - 2·sum
            double got_val = decrypt_val(result);
            
            // Decode: malapit sa φ² → 1, malapit sa 0 → 0
            int got_bit = (std::abs(got_val - PHI_SQ) < 1.0) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (val=" << got_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << result->GetLevel() << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE NAND PERFECT!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
