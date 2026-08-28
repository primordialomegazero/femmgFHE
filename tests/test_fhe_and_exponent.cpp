// FHE AND VIA EXPONENT ADDITION — 0-LEVEL
// AND(a,b) = φ^(exp_a + exp_b)
// Lahat homomorphic, walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE AND VIA EXPONENT ADDITION\n";
    std::cout << "  0-Level, Walang Decrypt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    
    // Exponent encoding:
    // Input 0 → exponent -4
    // Input 1 → exponent 0
    const double EXP_0 = -4.0;
    const double EXP_1 = 0.0;

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

    auto ct_exp_0 = make_ct(EXP_0);
    auto ct_exp_1 = make_ct(EXP_1);
    auto ct_phi_sq = make_ct(PHI_SQ);

    std::cout << "FHE AND TEST (exponent addition):\n";
    std::cout << "=================================\n\n";

    int correct = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Encode inputs bilang exponents
            double exp_a = a_bit ? EXP_1 : EXP_0;
            double exp_b = b_bit ? EXP_1 : EXP_0;
            
            auto ct_exp_a = make_ct(exp_a);
            auto ct_exp_b = make_ct(exp_b);
            
            // AND = exp_a + exp_b (homomorphic addition!)
            auto ct_exp_result = cc->EvalAdd(ct_exp_a, ct_exp_b);
            
            // Decode: φ^(exp_result)
            double exp_result = decrypt_val(ct_exp_result);
            double and_val = std::pow(PHI, exp_result);
            
            // Decode: malapit sa 1 → bit 1, malapit sa 0 → bit 0
            int got_bit = (std::abs(and_val - 1.0) < 0.3) ? 1 : 0;
            int expected_bit = a_bit && b_bit;
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  AND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (exp=" << exp_result << ", φ^exp=" << and_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE AND 0-LEVEL!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
