// HOMOMORPHIC THRESHOLD SA PERIOD-0 SPACE
// Hanapin kung may natural na paraan para sa > 0.2
// nang walang decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HOMOMORPHIC THRESHOLD SEARCH\n";
    std::cout << "  Natural > 0.2 sa Period-0 Space\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double PHI2_MOD = 0.38196601125010515;
    const double PHI3_MOD = 0.2360679774997897;
    const double PHI4_MOD = 0.8541019662496845;
    const double THRESHOLD = 0.2;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    auto ct_phi_mod = make_ct(PHI_MOD);
    auto ct_phi2_mod = make_ct(PHI2_MOD);
    auto ct_phi3_mod = make_ct(PHI3_MOD);
    auto ct_phi4_mod = make_ct(PHI4_MOD);
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_phi4_mod, sum);
    };

    // Subukan ang iba't ibang paraan para makuha ang threshold
    std::cout << "THRESHOLD METHODS:\n";
    std::cout << "==================\n\n";

    // Method 1: Direct comparison (sa plaintext lang ito)
    std::cout << "Method 1: Direct value + threshold\n";
    auto test_val = eval_nand(ct_phi_mod, ct_phi_mod);
    double v = decrypt_val(test_val);
    std::cout << "  Value: " << v << " → " << (v > THRESHOLD ? 1 : 0) << "\n\n";

    // Method 2: Polynomial approximation ng step function
    std::cout << "Method 2: Polynomial sign approximation\n";
    std::cout << "  Step(x) = 0.5 + 0.5*tanh(k*x)\n";
    std::cout << "  Hindi pa homomorphic kung walang EvalTanh\n\n";

    // Method 3: Natural φ separation
    std::cout << "Method 3: Natural φ separation\n";
    std::cout << "  φ³ = 0.236068 (malapit sa threshold)\n";
    std::cout << "  φ⁴ = 0.854102 (malayo sa threshold)\n";
    std::cout << "  -φ² = -0.381966 (negative)\n\n";

    // Subukan: NAND output vs φ³
    auto nand_00 = eval_nand(ct_zero, ct_zero);      // 0.854102
    auto nand_01 = eval_nand(ct_zero, ct_phi_mod);   // 0.236068
    auto nand_11 = eval_nand(ct_phi_mod, ct_phi_mod); // -0.381966

    std::cout << "NAND outputs vs φ³:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n";
    std::cout << "  φ³ = " << PHI3_MOD << "\n\n";

    // Pagkakaiba sa φ³
    auto diff_00 = cc->EvalSub(nand_00, ct_phi3_mod);
    auto diff_01 = cc->EvalSub(nand_01, ct_phi3_mod);
    auto diff_11 = cc->EvalSub(nand_11, ct_phi3_mod);

    std::cout << "Differences from φ³:\n";
    std::cout << "  NAND(0,0) - φ³ = " << decrypt_val(diff_00) << "\n";
    std::cout << "  NAND(0,1) - φ³ = " << decrypt_val(diff_01) << "\n";
    std::cout << "  NAND(1,1) - φ³ = " << decrypt_val(diff_11) << "\n\n";

    // Sign ng difference: positive → 1, negative → 0
    std::cout << "Sign pattern:\n";
    std::cout << "  NAND(0,0): " << (decrypt_val(diff_00) > 0 ? "+" : "-") << " → 1\n";
    std::cout << "  NAND(0,1): " << (decrypt_val(diff_01) > 0 ? "+" : "-") << " → ?\n";
    std::cout << "  NAND(1,1): " << (decrypt_val(diff_11) > 0 ? "+" : "-") << " → 0\n\n";

    // Method 4: Squaring the difference
    std::cout << "Method 4: Squared difference (homomorphic mult)\n";
    auto sq_00 = cc->EvalMult(diff_00, diff_00);
    auto sq_01 = cc->EvalMult(diff_01, diff_01);
    auto sq_11 = cc->EvalMult(diff_11, diff_11);

    std::cout << "  (NAND(0,0) - φ³)² = " << decrypt_val(sq_00) << "\n";
    std::cout << "  (NAND(0,1) - φ³)² = " << decrypt_val(sq_01) << "\n";
    std::cout << "  (NAND(1,1) - φ³)² = " << decrypt_val(sq_11) << "\n";
    std::cout << "  Level after mult: " << sq_00->GetLevel() << "\n\n";

    return 0;
}
