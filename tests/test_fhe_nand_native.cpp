// FHE NAND — NATIVE φ-DOMAIN
// Walang decrypt sa gitna
// Pure CKKS operations

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND — NATIVE φ-DOMAIN\n";
    std::cout << "  Walang Decrypt sa Gitna\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_MOD = PHI_SQ - 2.0;  // 0.618034

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    // φ-domain constants
    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_phi_mod = make_ct(PHI_MOD);
    auto ct_zero = make_ct(0.0);

    std::cout << "FHE NAND TEST (encrypted inputs):\n";
    std::cout << "================================\n\n";

    int correct = 0;
    Ciphertext<DCRTPoly> result;  // Declared sa labas!

    // Test lahat ng 4 combinations
    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Encode inputs sa φ-domain
            double a_val = a_bit ? PHI_SQ : 0.0;
            double b_val = b_bit ? PHI_SQ : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // NAND via φ-domain (walang decrypt!)
            // Formula: φ² - (a+b) + φ_mod
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            auto neg_sum = cc->EvalSub(ct_zero, sum_ab);
            result = cc->EvalAdd(ct_phi_sq, neg_sum);
            
            // Correction para sa (0,0) case
            auto corrected = cc->EvalAdd(result, ct_phi_mod);
            
            double got_val = decrypt_val(corrected);
            
            // Decode: φ² o malapit → bit 1, otherwise → bit 0
            int got_bit = (std::abs(got_val - PHI_SQ) < 0.5) ? 1 : 0;
            
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
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE NAND WORKS!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
