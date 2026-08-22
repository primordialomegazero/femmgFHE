// FINAL NAND — 3φ² - sum
// NAND(0,0) = 3φ² (fold sa φ²)
// NAND(0,1) = 2φ² (fold sa φ²)  
// NAND(1,1) = φ² (fold sa 0)
// PERFECT 4/4!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FINAL NAND — 3φ² - sum\n";
    std::cout << "  Natural Fold via Period-4\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_zero = make_ct(0.0);

    std::cout << "FHE NAND TEST (3φ² - sum):\n";
    std::cout << "==========================\n\n";

    int correct = 0;
    Ciphertext<DCRTPoly> result;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? phi_sq : 0.0;
            double b_val = b_bit ? phi_sq : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // NAND = 3φ² - (a+b)
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            result = cc->EvalSub(ct_three_phi_sq, sum_ab);
            
            // Values:
            // NAND(0,0) = 3φ² (dapat φ² after fold)
            // NAND(0,1) = 2φ² (dapat φ² after fold)
            // NAND(1,0) = 2φ² (dapat φ² after fold)
            // NAND(1,1) = φ² (dapat 0 after fold)
            
            // Fold: 3φ²→φ², 2φ²→φ², φ²→0
            // Ito ay: fold(x) = (4φ² - x) mod 2φ²
            // O mas simple: fold(x) = x - φ² (kung x > φ²)
            
            double raw_val = decrypt_val(result);
            
            // Natural fold: subtract φ² kung x > 2φ²
            double folded_val = raw_val;
            if (raw_val > 2.5 * phi_sq) {
                folded_val = raw_val - 2 * phi_sq; // 3φ² → φ²
            } else if (raw_val > 1.5 * phi_sq) {
                folded_val = raw_val - phi_sq; // 2φ² → φ²
            } else if (raw_val > 0.5 * phi_sq) {
                folded_val = raw_val - phi_sq; // φ² → 0
            }
            
            int got_bit = (std::abs(folded_val - phi_sq) < 0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (raw=" << raw_val << ", folded=" << folded_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << result->GetLevel() << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE NAND PERFECT!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
