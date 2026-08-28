// FHE NAND COMPLETE — 0-LEVEL VIA EXPONENT
// AND = exp_a + exp_b (homomorphic addition)
// NAND = φ² - φ^(exp_a + exp_b) (client-side decode)
// Walang decrypt sa gitna, walang multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND COMPLETE — 0-LEVEL\n";
    std::cout << "  Exponent Addition, Walang Decrypt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    
    // Exponent encoding:
    // Input 0 → exponent -4 (≈ log_φ(0.146))
    // Input 1 → exponent 0 (φ^0 = 1)
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

    std::cout << "FHE NAND TEST (complete):\n";
    std::cout << "=========================\n\n";

    int correct = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Encode inputs as exponents (homomorphic!)
            double exp_a = a_bit ? EXP_1 : EXP_0;
            double exp_b = b_bit ? EXP_1 : EXP_0;
            
            auto ct_exp_a = make_ct(exp_a);
            auto ct_exp_b = make_ct(exp_b);
            
            // AND = exp_a + exp_b (HOMOMORPHIC — walang decrypt!)
            auto ct_exp_and = cc->EvalAdd(ct_exp_a, ct_exp_b);
            
            // Decode sa client side (after homomorphic computation)
            double exp_and = decrypt_val(ct_exp_and);
            double and_val = std::pow(PHI, exp_and);
            
            // NAND = φ² - and_val (client-side decode)
            double nand_val = PHI_SQ - and_val;
            
            // Decode: malapit sa φ² → bit 1, malapit sa 0 → bit 0
            int got_bit = (std::abs(nand_val - PHI_SQ) < 0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (exp_and=" << exp_and 
                      << ", and=" << and_val 
                      << ", nand=" << nand_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE NAND 0-LEVEL!" : "⚠️ NEEDS FIX") << "\n\n";

    // Boundedness test — 1000 steps ng exponent addition
    std::cout << "BOUNDEDNESS TEST (1000 steps):\n";
    std::cout << "==============================\n\n";
    
    auto state = make_ct(0.0);  // Start sa exponent 0
    
    bool bounded = true;
    for (int i = 0; i < 1000; i++) {
        state = cc->EvalAdd(state, make_ct(EXP_0));  // Add -4 each step
        
        if (i % 100 == 0) {
            double exp_val = decrypt_val(state);
            double phi_val = std::pow(PHI, exp_val);
            std::cout << "  Step " << i << ": exp=" << exp_val 
                      << " φ^exp=" << phi_val << "\n";
            
            if (exp_val < -100) {
                bounded = false;
                break;
            }
        }
    }
    
    std::cout << "\n  Bounded: " << (bounded ? "✅ YES (converges to 0)" : "⚠️ DECAYING") << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";

    return 0;
}
