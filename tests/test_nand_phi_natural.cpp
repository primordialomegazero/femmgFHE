// NAND — Natural φ Encoding
// 0 → 0, 1 → π - φ
// Threshold: cos > -0.5 → 1
//
// ANG SUSI: φ = 2cos(π/5)
// π - φ = π - 1.618 = 1.524 radians
// cos(π - φ) = -cos(φ) = +0.047 (positive!)
// cos(2π - 2φ) = cos(2φ) = -0.996 (strongly negative!)
//
// NAND(0,0) = cos(0) = 1 → 1 ✓
// NAND(0,1) = cos(π-φ) = +0.047 → 1 ✓
// NAND(1,0) = cos(π-φ) = +0.047 → 1 ✓
// NAND(1,1) = cos(2π-2φ) = -0.996 → 0 ✓
//
// LAHAT ADDITIVE! Walang EvalMult!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND — Natural φ Encoding\n";
    std::cout << "  Additive Only, 0-Level\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    const double ENC_1 = PI - phi;  // 1.524 radians

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

    std::cout << "ENCODING: 0 → 0, 1 → π - φ = " << ENC_1 << "\n";
    std::cout << "THRESHOLD: cos > -0.5 → 1\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double angle_a = a_bit ? ENC_1 : 0.0;
            double angle_b = b_bit ? ENC_1 : 0.0;
            
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            
            // NAND = (a + b) — walang phase shift!
            auto result = cc->EvalAdd(ct_a, ct_b);
            
            double val = decrypt_val(result);
            double cos_val = std::cos(val);
            
            // Natural threshold: cos > -0.5 → 1
            int got_bit = (cos_val > -0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (angle=" << val << ", cos=" << cos_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = result->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << " (0 = additive!)\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ NAND 0-LEVEL!" : "⚠️ NEEDS FIX") << "\n";
    
    // Verify the math
    std::cout << "\n  VERIFICATION:\n";
    std::cout << "  cos(0) = " << std::cos(0) << "\n";
    std::cout << "  cos(π-φ) = " << std::cos(ENC_1) << "\n";
    std::cout << "  cos(2(π-φ)) = " << std::cos(2*ENC_1) << "\n";
    std::cout << "  φ = 2cos(π/5) = " << 2*std::cos(PI/5) << "\n";

    return 0;
}
