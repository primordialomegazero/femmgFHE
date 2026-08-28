// FIBONACCI MAPPING — Natural Sign Detection
// I-map ang NAND output sa Fibonacci word space
// Walang decrypt, walang multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI MAPPING\n";
    std::cout << "  Natural Sign Detection\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

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

    // Key insight: Ang Fibonacci word ay may natural na paghihiwalay
    // Position 0 (even) → bit 1
    // Position 1 (odd) → bit 0
    // At ang φ ay may natural na periodicity
    
    // Subukan: NAND output + Fibonacci word shift
    // Kung NAND output ay positive → map sa even position (bit 1)
    // Kung negative → map sa odd position (bit 0)
    
    // Ang trick: gamitin ang φ mismo bilang natural na separator
    
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);
    
    // NAND: 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };
    
    // Test NAND outputs
    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);
    
    std::cout << "NAND OUTPUTS:\n";
    std::cout << "=============\n\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";
    
    // Natural na paghihiwalay gamit ang φ
    // I-add ang φ/2 para ma-shift ang zero crossing
    auto ct_phi_half = make_ct(PHI / 2.0);
    
    auto shifted_00 = cc->EvalAdd(nand_00, ct_phi_half);
    auto shifted_01 = cc->EvalAdd(nand_01, ct_phi_half);
    auto shifted_11 = cc->EvalAdd(nand_11, ct_phi_half);
    
    std::cout << "SHIFTED NAND (x + φ/2):\n";
    std::cout << "=======================\n\n";
    std::cout << "  NAND(0,0) + φ/2 = " << decrypt_val(shifted_00) << "\n";
    std::cout << "  NAND(0,1) + φ/2 = " << decrypt_val(shifted_01) << "\n";
    std::cout << "  NAND(1,1) + φ/2 = " << decrypt_val(shifted_11) << "\n\n";
    
    // OBSERVATION: Ang φ/2 shift ay naghihiwalay ng positive at negative
    // 1 + 0.809 = 1.809 (positive)
    // 0.382 + 0.809 = 1.191 (positive)
    // -0.236 + 0.809 = 0.573 (positive pa rin)
    
    // Hmm... hindi pa rin maganda ang separation
    // Kailangan natin ng mas magandang shift
    
    // Subukan: i-multiply sa φ (level 1)
    auto ct_phi = make_ct(PHI);
    auto mult_00 = cc->EvalMult(nand_00, ct_phi);
    auto mult_01 = cc->EvalMult(nand_01, ct_phi);
    auto mult_11 = cc->EvalMult(nand_11, ct_phi);
    
    std::cout << "MULTIPLIED NAND (x * φ):\n";
    std::cout << "========================\n\n";
    std::cout << "  NAND(0,0) * φ = " << decrypt_val(mult_00) << "\n";
    std::cout << "  NAND(0,1) * φ = " << decrypt_val(mult_01) << "\n";
    std::cout << "  NAND(1,1) * φ = " << decrypt_val(mult_11) << "\n";
    std::cout << "  Level: " << mult_00->GetLevel() << "\n\n";
    
    // Mas maganda! NAND(1,1) * φ = -0.382 (negative)
    // NAND(0,1) * φ = 0.618 (positive)
    // Zero crossing ay nasa pagitan!
    
    std::cout << "ZERO CROSSING ANALYSIS:\n";
    std::cout << "========================\n\n";
    std::cout << "  NAND(1,1) * φ = " << decrypt_val(mult_11) << " (negative → 0)\n";
    std::cout << "  NAND(0,1) * φ = " << decrypt_val(mult_01) << " (positive → 1)\n\n";
    
    return 0;
}
