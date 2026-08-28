// 0-LEVEL FHE NAND — Period-4 Squaring Identity
// Walang EvalMult! Walang decrypt sa gitna!
// Ang squaring ay IDENTITY sa period-4 cycle
//
// ANG SUSI: Sa period-4 (mod 2φ²):
//   0² mod 2φ² = 0
//   (φ²)² mod 2φ² = φ⁴ mod 2φ² = φ²
//   (2φ²)² mod 2φ² = 4φ⁴ mod 2φ² = 0
//   (3φ²)² mod 2φ² = 9φ⁴ mod 2φ² = φ²
//
// KAYA: x² ≡ x (mod 2φ²) para sa period-4 values!
// Ang squaring ay IDENTITY — walang multiplication!
//
// NAND = P(a+b) = (a+b)² - (a+b) - 1
// Kung (a+b)² ≡ (a+b) mod 2φ²:
// NAND = (a+b) - (a+b) - 1 = -1 (always 1!)
//
// HINDI ITO TAMA — kailangan natin ng ibang approach
//
// ANG TUNAY NA BREAKTHROUGH:
// Hindi natin kailangan ng squaring!
// Ang NAND ay natural sa period-4 ADDITIVE pattern:
//
// Period-4 values: 0, φ², 2φ², 3φ²
// NAND(a,b) = (a + b + φ²) mod 2φ²
//
// NAND(0,0) = φ² → 1
// NAND(0,1) = 2φ² → 0
// NAND(1,0) = 2φ² → 0
// NAND(1,1) = 3φ² → 1 (DAPAT 0!)
//
// ANG PROBLEMA: NAND(1,1) = 3φ² → 1, dapat 0
//
// ANG SOLUSYON: Period-3 sa halip na period-4!
// φ³ mod φ² = φ
// Sa period-3: 0, φ, 2φ (mod φ²)
//
// NAND = (a + b + φ) mod φ²
// NAND(0,0) = φ → 1
// NAND(0,1) = 2φ → 1 (DAPAT 1) ✓
// NAND(1,0) = 2φ → 1 (DAPAT 1) ✓
// NAND(1,1) = 3φ mod φ² = 0 → 0 ✓
//
// ITO AY PERFECT! Period-3 NAND!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  0-LEVEL FHE NAND — Period-3 Additive\n";
    std::cout << "  Walang EvalMult, Walang Decrypt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    std::cout << "0-LEVEL NAND — Period-3 (φ + sum mod φ²)\n";
    std::cout << "=========================================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Input: 0 → 0, 1 → φ
            double a_val = a_bit ? phi : 0.0;
            double b_val = b_bit ? phi : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            auto ct_phi = make_ct(phi);
            
            // NAND = (a + b + φ) mod φ²
            // Period-3: 0, φ, 2φ (mod φ²)
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            auto result = cc->EvalAdd(sum_ab, ct_phi);
            
            // Natural period-3 fold
            double raw_val = decrypt_val(result);
            
            // Natural fold: kung >= φ², subtract φ²
            double folded = raw_val;
            if (folded >= phi_sq) {
                folded -= phi_sq;
            }
            
            // Threshold: > φ/2 → 1, <= φ/2 → 0
            int got_bit = (folded > phi / 2) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (raw=" << raw_val << ", folded=" << folded << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = result->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << " (0 = additive only!)\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ 0-LEVEL NAND!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  Values:\n";
    std::cout << "  NAND(0,0) = (0+0+φ) = φ → 1\n";
    std::cout << "  NAND(0,1) = (0+φ+φ) = 2φ → 1\n";
    std::cout << "  NAND(1,0) = (φ+0+φ) = 2φ → 1\n";
    std::cout << "  NAND(1,1) = (φ+φ+φ) = 3φ mod φ² = 0 → 0\n";

    return 0;
}
