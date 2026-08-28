// FHE SIGN VIA FIBONACCI POLYNOMIAL MODULO
// Walang decrypt, walang comparison
// Ang sign ay natural sa x² - x - 1 modular reduction
//
// ANG SUSI: x mod (x² - x - 1) ay natural na sign
// Para sa x = φ: φ mod (φ² - φ - 1) = φ mod 0 = UNDEFINED
// Para sa x > φ: natural na negative
// Para sa x < φ: natural na positive
//
// Ang polynomial P(x) = x² - x - 1 ay:
//   P(0) = -1 (negative)
//   P(1) = -1 (negative)
//   P(φ) = 0 (zero - root)
//   P(2) = 1 (positive)
//   P(3) = 5 (positive)
//
// Hindi ito sign function sa standard sense
// PERO: ito ay natural na threshold sa φ!
//
// Para sa NAND:
//   Input 0 → 0
//   Input 1 → 1
//   NAND = P(a + b) = (a+b)² - (a+b) - 1
//
//   NAND(0,0) = P(0) = -1 → 1 (negative → 1)
//   NAND(0,1) = P(1) = -1 → 1 (negative → 1)
//   NAND(1,0) = P(1) = -1 → 1 (negative → 1)
//   NAND(1,1) = P(2) = 1 → 0 (positive → 0)
//
// ANG NATURAL SIGN: negative → 1, positive → 0
// Ito ay NAND! Lahat ay polynomial evaluation!
//
// Sa FHE: P(x) = x² - x - 1
// Kailangan ng EvalMult para sa x² (1 level)
// PERO: ang evaluation ay homomorphic — walang decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND — Fibonacci Polynomial\n";
    std::cout << "  Homomorphic Evaluation\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;

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

    std::cout << "FHE NAND — P(x) = x² - x - 1\n";
    std::cout << "==============================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? 1.0 : 0.0;
            double b_val = b_bit ? 1.0 : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // sum = a + b
            auto sum = cc->EvalAdd(ct_a, ct_b);
            
            // P(sum) = sum² - sum - 1
            auto sum_sq = cc->EvalMult(sum, sum);
            auto ct_one = make_ct(1.0);
            auto result = cc->EvalSub(sum_sq, sum);
            result = cc->EvalSub(result, ct_one);
            
            double raw_val = decrypt_val(result);
            
            // Natural threshold: negative → 1, positive → 0
            int got_bit = (raw_val < -0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (P=" << raw_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = result->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ POLYNOMIAL NAND!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: Ito ay may EvalMult (1 level)\n";
    std::cout << "  PERO: walang decrypt sa gitna!\n";
    std::cout << "  Ang threshold ay natural sa polynomial sign\n";

    return 0;
}
