// FHE NAND — Natural Modulo via CKKS Scaling
// Walang EvalMult, Walang Decrypt sa gitna
// Ang modulo ay natural na naka-encode
//
// ANG SUSI: CKKS scaling modulus
// Kung ang scaling factor ay φ², ang values na lumampas
// ay natural na mag-wrap-around
//
// Input: 0 → 0, 1 → φ²
// NAND = (a + b + φ²) mod 2φ²
//
// Sa CKKS: EvalAdd lamang
// Ang natural modulo ay nasa encoding structure

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND — Natural Modulo Encoding\n";
    std::cout << "  Additive Only, No EvalMult\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
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

    std::cout << "FHE NAND — Natural Modulo (Additive Only)\n";
    std::cout << "=========================================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? phi_sq : 0.0;
            double b_val = b_bit ? phi_sq : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            auto ct_phi_sq = make_ct(phi_sq);
            
            // NAND = (a + b + φ²) mod 2φ²
            // Natural modulo via CKKS overflow
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto shifted = cc->EvalAdd(sum, ct_phi_sq);
            
            // Ang natural modulo ay mangyayari kapag ang value
            // ay lumampas sa 2φ² — automatic wrap-around
            // sa CKKS plaintext scaling
            
            double raw_val = decrypt_val(shifted);
            
            // Natural threshold: > φ²/2 → 1, < φ²/2 → 0
            int got_bit = (raw_val > phi_sq / 2) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (raw=" << raw_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = shifted->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ NATURAL MOD NAND!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  Values:\n";
    std::cout << "  NAND(0,0) = (0+0+φ²) = φ² → 1\n";
    std::cout << "  NAND(0,1) = (0+φ²+φ²) = 2φ² → 0\n";
    std::cout << "  NAND(1,0) = (φ²+0+φ²) = 2φ² → 0\n";
    std::cout << "  NAND(1,1) = (φ²+φ²+φ²) = 3φ² → 1\n";
    std::cout << "\n  PROBLEMA: NAND(1,1) dapat 0 pero lumalabas 1\n";
    std::cout << "  Kailangan natin ng DIFFERENT phase shift\n";

    return 0;
}
