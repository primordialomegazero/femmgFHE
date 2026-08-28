// CKKS HOMOMORPHIC THRESHOLD — Addition Only
// Hanapin ang level-0 na paraan para sa sign detection
// Walang multiplication, walang decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS HOMOMORPHIC THRESHOLD\n";
    std::cout << "  Addition Only, Level 0\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;

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

    // Test values: -0.236 (NAND 1,1) at 0.382 (NAND 0,1)
    auto test_neg = make_ct(-0.236068);
    auto test_pos = make_ct(0.381966);

    // Natural threshold candidates (addition only)
    std::cout << "THRESHOLD CANDIDATES (Addition Only):\n";
    std::cout << "=====================================\n\n";

    // 1. x + φ — shift para makita ang sign
    auto ct_phi = make_ct(PHI);
    auto shift1_neg = cc->EvalAdd(test_neg, ct_phi);
    auto shift1_pos = cc->EvalAdd(test_pos, ct_phi);
    
    std::cout << "1. x + φ:\n";
    std::cout << "   -0.236 + φ = " << decrypt_val(shift1_neg) << "\n";
    std::cout << "   0.382 + φ = " << decrypt_val(shift1_pos) << "\n";
    std::cout << "   Level: " << shift1_neg->GetLevel() << "\n\n";

    // 2. x - 1/φ — shift para makita ang sign
    auto ct_phi_inv = make_ct(PHI_INV);
    auto shift2_neg = cc->EvalSub(test_neg, ct_phi_inv);
    auto shift2_pos = cc->EvalSub(test_pos, ct_phi_inv);
    
    std::cout << "2. x - 1/φ:\n";
    std::cout << "   -0.236 - 1/φ = " << decrypt_val(shift2_neg) << "\n";
    std::cout << "   0.382 - 1/φ = " << decrypt_val(shift2_pos) << "\n";
    std::cout << "   Level: " << shift2_neg->GetLevel() << "\n\n";

    // 3. x + φ² — larger shift
    auto ct_phi_sq = make_ct(PHI_SQ);
    auto shift3_neg = cc->EvalAdd(test_neg, ct_phi_sq);
    auto shift3_pos = cc->EvalAdd(test_pos, ct_phi_sq);
    
    std::cout << "3. x + φ²:\n";
    std::cout << "   -0.236 + φ² = " << decrypt_val(shift3_neg) << "\n";
    std::cout << "   0.382 + φ² = " << decrypt_val(shift3_pos) << "\n";
    std::cout << "   Level: " << shift3_neg->GetLevel() << "\n\n";

    // 4. x - φ — negative shift
    auto shift4_neg = cc->EvalSub(test_neg, ct_phi);
    auto shift4_pos = cc->EvalSub(test_pos, ct_phi);
    
    std::cout << "4. x - φ:\n";
    std::cout << "   -0.236 - φ = " << decrypt_val(shift4_neg) << "\n";
    std::cout << "   0.382 - φ = " << decrypt_val(shift4_pos) << "\n";
    std::cout << "   Level: " << shift4_neg->GetLevel() << "\n\n";

    // 5. x + x — double (addition lang)
    auto double_neg = cc->EvalAdd(test_neg, test_neg);
    auto double_pos = cc->EvalAdd(test_pos, test_pos);
    
    std::cout << "5. 2x:\n";
    std::cout << "   2(-0.236) = " << decrypt_val(double_neg) << "\n";
    std::cout << "   2(0.382) = " << decrypt_val(double_pos) << "\n";
    std::cout << "   Level: " << double_neg->GetLevel() << "\n\n";

    // 6. x - x = 0 — self-cancellation
    auto cancel_neg = cc->EvalSub(test_neg, test_neg);
    auto cancel_pos = cc->EvalSub(test_pos, test_pos);
    
    std::cout << "6. x - x:\n";
    std::cout << "   -0.236 - (-0.236) = " << decrypt_val(cancel_neg) << "\n";
    std::cout << "   0.382 - 0.382 = " << decrypt_val(cancel_pos) << "\n";
    std::cout << "   Level: " << cancel_neg->GetLevel() << "\n\n";

    // 7. φ - x — reverse
    auto rev_neg = cc->EvalSub(ct_phi, test_neg);
    auto rev_pos = cc->EvalSub(ct_phi, test_pos);
    
    std::cout << "7. φ - x:\n";
    std::cout << "   φ - (-0.236) = " << decrypt_val(rev_neg) << "\n";
    std::cout << "   φ - 0.382 = " << decrypt_val(rev_pos) << "\n";
    std::cout << "   Level: " << rev_neg->GetLevel() << "\n\n";

    return 0;
}
