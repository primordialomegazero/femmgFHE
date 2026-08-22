// FHE NAND VIA MIN — UNIVERSAL GATE
// AND = min(a,b), NAND = φ² - min(a,b)
// Walang decrypt, 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND VIA MIN\n";
    std::cout << "  Universal Gate, 0-Level\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;

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
    auto ct_zero = make_ct(0.0);

    std::cout << "FHE NAND VIA MIN TEST:\n";
    std::cout << "======================\n\n";

    int correct = 0;
    Ciphertext<DCRTPoly> result;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? PHI_SQ : 0.0;
            double b_val = b_bit ? PHI_SQ : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // AND = min(a,b) = (a+b - |a-b|)/2
            // Sa φ-domain, |a-b| ay maaaring i-approximate
            // Para sa binary: |a-b| = a+b (kung isa lang ang non-zero)
            // O = 0 (kung pareho)
            
            // Simpleng approximation: AND ≈ a+b (kung pareho sila)
            // Pero kailangan natin ng exact
            
            // Subukan: NAND = φ² - min(a,b)
            // min(0,0)=0 → NAND=φ² ✓
            // min(0,φ²)=0 → NAND=φ² ✓
            // min(φ²,φ²)=φ² → NAND=0 ✓
            
            // Para sa binary, min(a,b) = a·b/φ² (kailangan ng mult)
            // O: min(a,b) = (a+b) - φ² (kung a=b=φ²)
            
            // Simpleng 0-level attempt:
            // Kung a+b = 0 → min = 0
            // Kung a+b = φ² → min = 0 (kasi isa lang)
            // Kung a+b = 2φ² → min = φ² (pareho)
            
            // NAND = φ² - (a+b-φ² kung a+b>φ²)
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            
            // Subukan: NAND = φ² - sum_ab + φ² (kung sum > φ²)
            // Simplified: NAND = 2φ² - sum_ab (para sa sum=2φ² → 0)
            auto two_phi_sq = cc->EvalAdd(ct_phi_sq, ct_phi_sq);
            result = cc->EvalSub(two_phi_sq, sum_ab);
            
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
