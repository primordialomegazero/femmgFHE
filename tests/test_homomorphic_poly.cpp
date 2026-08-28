// HOMOMORPHIC THRESHOLD — POLYNOMIAL APPROXIMATION
// Step function via φ-polynomial
// May level cost pero homomorphic

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HOMOMORPHIC POLY THRESHOLD\n";
    std::cout << "  φ-Polynomial Step Approximation\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double PHI3_MOD = 0.2360679774997897;
    const double PHI4_MOD = 0.8541019662496845;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_one = make_ct(1.0);

    // NAND(a,b) = φ⁴ - (a+b)
    auto nand_raw = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto ct_phi4 = make_ct(PHI4_MOD);
        return cc->EvalSub(ct_phi4, sum);
    };

    // Homomorphic threshold: step(x) = 1 kung x > 0, 0 kung x < 0
    // Approximation: step(x) ≈ 0.5 + 0.5·x/√(x²+ε)
    // O polynomial: step(x) ≈ 0.5 + 0.5·(3x - x³)/2 (Taylor)
    
    auto threshold_poly = [&](auto x) {
        // 3rd order approximation ng sign function
        // sign(x) ≈ 1.5x - 0.5x³ (sa range [-1,1])
        auto x2 = cc->EvalMult(x, x);        // x²
        auto x3 = cc->EvalMult(x2, x);       // x³
        
        auto ct_1_5 = make_ct(1.5);
        auto ct_0_5 = make_ct(0.5);
        
        auto term1 = cc->EvalMult(ct_1_5, x);   // 1.5x
        auto term2 = cc->EvalMult(ct_0_5, x3);  // 0.5x³
        
        return cc->EvalSub(term1, term2);
    };

    std::cout << "HOMOMORPHIC POLY THRESHOLD TEST:\n";
    std::cout << "=================================\n\n";

    auto ct_zero = make_ct(0.0);
    auto ct_phi_mod = make_ct(PHI_MOD);

    // NAND results (raw)
    auto n00 = nand_raw(ct_zero, ct_zero);
    auto n01 = nand_raw(ct_zero, ct_phi_mod);
    auto n11 = nand_raw(ct_phi_mod, ct_phi_mod);

    // Apply polynomial threshold
    auto t00 = threshold_poly(n00);
    auto t01 = threshold_poly(n01);
    auto t11 = threshold_poly(n11);

    std::cout << "  NAND(0,0) = " << decrypt_val(n00) 
              << " → threshold → " << decrypt_val(t00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(n01) 
              << " → threshold → " << decrypt_val(t01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(n11) 
              << " → threshold → " << decrypt_val(t11) << "\n\n";

    std::cout << "  Level after threshold: " << t00->GetLevel() << "\n";
    std::cout << "  (May level cost pero homomorphic)\n";

    return 0;
}
