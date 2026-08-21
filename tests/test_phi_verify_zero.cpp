// I-VERIFY: BAKIT LAHAT 0?
// Test natin ang alternating pattern manually

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  VERIFY: ALTERNATING PATTERN TEST\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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

    auto ct_phi = make_ct(phi);
    auto ct_one = make_ct(1.0);
    auto ct_zero = make_ct(0.0);

    std::cout << "MANUAL TEST (Depth 10):\n";
    std::cout << "=======================\n\n";

    // Test 1: NAND(φ, φ·ψ) = φ - φ·1 = 0
    auto prod1 = cc->EvalMult(ct_phi, ct_one);
    auto result1 = cc->EvalSub(ct_phi, prod1);
    std::cout << "NAND(φ, φ·ψ) = " << decrypt_val(result1) 
              << " (expected 0)\n";

    // Test 2: NAND(0, φ·ψ) = φ - 0·1 = φ
    auto prod2 = cc->EvalMult(ct_zero, ct_one);
    auto result2 = cc->EvalSub(ct_phi, prod2);
    std::cout << "NAND(0, φ·ψ) = " << decrypt_val(result2) 
              << " (expected φ=" << phi << ")\n";

    // Test 3: Alternating sequence
    std::cout << "\nAlternating sequence:\n";
    auto current = ct_phi;
    
    for (int i = 0; i < 10; i++) {
        auto prod = cc->EvalMult(current, ct_one);
        current = cc->EvalSub(ct_phi, prod);
        
        std::cout << "  Step " << i << ": v=" << decrypt_val(current);
        if (i % 2 == 0) std::cout << " (expected 0)";
        else std::cout << " (expected φ)";
        std::cout << "\n";
    }

    return 0;
}
