// EMERGENT HOMOMORPHIC THRESHOLD — Golden Ratio Deep Dive
// Walang EvalExp — gamitin ang natural na φ properties

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT THRESHOLD — φ DEEP DIVE\n";
    std::cout << "  Natural Homomorphic Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
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

    std::cout << "EMERGENT THRESHOLD CANDIDATES:\n";
    std::cout << "==============================\n\n";

    // Values natin: -0.236 (NAND 1,1) at 0.382 (NAND 0,1)
    // Kailangan: paghiwalayin sila nang natural

    // 1. φ^2 * x — scaling na galing sa φ
    auto ct_phi_sq = make_ct(PHI * PHI);
    
    // Test: φ² * (-0.236) at φ² * (0.382)
    auto test_neg = make_ct(-0.236068);
    auto test_pos = make_ct(0.381966);
    
    auto scaled_neg = cc->EvalMult(test_neg, ct_phi_sq);
    auto scaled_pos = cc->EvalMult(test_pos, ct_phi_sq);
    
    std::cout << "1. SCALING: φ² * x\n";
    std::cout << "   φ² * (-0.236) = " << decrypt_val(scaled_neg) << "\n";
    std::cout << "   φ² * (0.382) = " << decrypt_val(scaled_pos) << "\n";
    std::cout << "   Level: " << scaled_neg->GetLevel() << "\n\n";

    // 2. x - 1/φ² — shift gamit ang φ power
    auto ct_phi_inv_sq = make_ct(1.0 / (PHI * PHI));
    auto shift2_neg = cc->EvalSub(test_neg, ct_phi_inv_sq);
    auto shift2_pos = cc->EvalSub(test_pos, ct_phi_inv_sq);
    
    std::cout << "2. SHIFT: x - 1/φ²\n";
    std::cout << "   -0.236 - 1/φ² = " << decrypt_val(shift2_neg) << "\n";
    std::cout << "   0.382 - 1/φ² = " << decrypt_val(shift2_pos) << "\n";
    std::cout << "   Level: " << shift2_neg->GetLevel() << "\n\n";

    // 3. x + φ — shift gamit ang φ
    auto ct_phi = make_ct(PHI);
    auto shift3_neg = cc->EvalAdd(test_neg, ct_phi);
    auto shift3_pos = cc->EvalAdd(test_pos, ct_phi);
    
    std::cout << "3. SHIFT: x + φ\n";
    std::cout << "   -0.236 + φ = " << decrypt_val(shift3_neg) << "\n";
    std::cout << "   0.382 + φ = " << decrypt_val(shift3_pos) << "\n";
    std::cout << "   Level: " << shift3_neg->GetLevel() << "\n\n";

    // 4. x - ψ — shift gamit ang ψ
    auto ct_psi = make_ct(PSI);
    auto shift4_neg = cc->EvalSub(test_neg, ct_psi);
    auto shift4_pos = cc->EvalSub(test_pos, ct_psi);
    
    std::cout << "4. SHIFT: x - ψ\n";
    std::cout << "   -0.236 - ψ = " << decrypt_val(shift4_neg) << "\n";
    std::cout << "   0.382 - ψ = " << decrypt_val(shift4_pos) << "\n";
    std::cout << "   Level: " << shift4_neg->GetLevel() << "\n\n";

    // 5. Natural sign via EvalSin
    std::cout << "5. EVALSIN SIGN DETECTION:\n";
    auto sin_neg = cc->EvalSin(test_neg, -4.0, 4.0, 5);
    auto sin_pos = cc->EvalSin(test_pos, -4.0, 4.0, 5);
    std::cout << "   sin(-0.236) = " << decrypt_val(sin_neg) << "\n";
    std::cout << "   sin(0.382) = " << decrypt_val(sin_pos) << "\n";
    std::cout << "   Level: " << sin_neg->GetLevel() << "\n\n";

    // 6. Natural sign via EvalCos
    std::cout << "6. EVALCOS SIGN DETECTION:\n";
    auto cos_neg = cc->EvalCos(test_neg, -4.0, 4.0, 5);
    auto cos_pos = cc->EvalCos(test_pos, -4.0, 4.0, 5);
    std::cout << "   cos(-0.236) = " << decrypt_val(cos_neg) << "\n";
    std::cout << "   cos(0.382) = " << decrypt_val(cos_pos) << "\n";
    std::cout << "   Level: " << cos_neg->GetLevel() << "\n\n";

    // 7. NAND raw + φ shift para sa positive/negative separation
    std::cout << "7. NAND + φ SHIFT:\n";
    auto nand_11_raw = make_ct(-0.236068);
    auto nand_01_raw = make_ct(0.381966);
    
    auto nand_11_shifted = cc->EvalAdd(nand_11_raw, ct_phi);
    auto nand_01_shifted = cc->EvalAdd(nand_01_raw, ct_phi);
    
    std::cout << "   NAND(1,1) + φ = " << decrypt_val(nand_11_shifted) << "\n";
    std::cout << "   NAND(0,1) + φ = " << decrypt_val(nand_01_shifted) << "\n";
    std::cout << "   Pareho positive — hindi gumagana\n\n";

    // 8. NAND raw - φ para sa sign flip
    auto nand_11_minus_phi = cc->EvalSub(nand_11_raw, ct_phi);
    auto nand_01_minus_phi = cc->EvalSub(nand_01_raw, ct_phi);
    
    std::cout << "8. NAND - φ SHIFT:\n";
    std::cout << "   NAND(1,1) - φ = " << decrypt_val(nand_11_minus_phi) << "\n";
    std::cout << "   NAND(0,1) - φ = " << decrypt_val(nand_01_minus_phi) << "\n";
    std::cout << "   Pareho negative — hindi gumagana\n\n";

    return 0;
}
