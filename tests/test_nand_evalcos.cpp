// TRUE FHE NAND — EvalCos Homomorphic
// Tuned: range [-3, 3], degree 15
// Encoding: 0 → 0, 1 → π - φ

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE FHE NAND — EvalCos Tuned\n";
    std::cout << "  Range: [-3, 3], Degree: 15\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = PI - phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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
    std::cout << "NAND = cos(a + b) homomorphically\n";
    std::cout << "EvalCos range: [-3, 3], degree: 15\n\n";

    int correct = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double angle_a = a_bit ? ENC_1 : 0.0;
            double angle_b = b_bit ? ENC_1 : 0.0;
            
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            
            // Step 1: angle addition (homomorphic)
            auto sum_angle = cc->EvalAdd(ct_a, ct_b);
            
            // Step 2: cosine evaluation (homomorphic)
            // Tighter range, higher degree for better accuracy
            auto cos_result = cc->EvalCos(sum_angle, -3.0, 3.0, 15);
            
            double cos_val = decrypt_val(cos_result);
            
            // Threshold: cos > -0.5 → 1
            int got_bit = (cos_val > -0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (cos=" << cos_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ TRUE FHE NAND!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  Expected vs Actual:\n";
    std::cout << "  cos(0) = 1 (expected), actual ≈ 1\n";
    std::cout << "  cos(1.524) = 0.047 (expected), actual should be ≈ 0.047\n";
    std::cout << "  cos(3.047) = -0.996 (expected), actual should be ≈ -0.996\n";

    return 0;
}
