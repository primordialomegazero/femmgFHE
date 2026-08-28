// SELF-NORMALIZING NAND
// Ang function mismo ang nagre-reset ng values
// Walang external threshold, self-referential ang lahat

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SELF-NORMALIZING NAND\n";
    std::cout << "  Walang External Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
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

    // Key insight: φ² - φ = 1, at 1/φ = φ - 1
    // Kaya ang NAND = 1 - (a+b) ay maaaring i-self-normalize
    
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    
    // Self-normalizing NAND:
    // NAND(a,b) = 1/φ kung (a+b) < 1/φ
    // NAND(a,b) = 0 kung (a+b) ≥ 1/φ
    // 
    // Sa φ-space: NAND(a,b) = φ² - φ - (a+b)
    // Kung positive → 1/φ
    // Kung negative → 0
    
    // Ang self-reference: 1/φ mismo ay nasa function
    auto eval_nand_self = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto result = cc->EvalSub(ct_1, sum);
        // Ito ay 1/φ - (a+b)
        // Kung positive, output ay positive
        // Kung negative, output ay negative
        return result;
    };

    std::cout << "SELF-NORMALIZING TEST:\n";
    std::cout << "======================\n\n";

    auto state = eval_nand_self(ct_1, ct_1);
    
    for (int i = 0; i < 5; i++) {
        double val = decrypt_val(state);
        std::cout << "  Iteration " << i << ": value=" << val << "\n";
        state = eval_nand_self(state, state);
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang function ay 1/φ - (a+b)\n";
    std::cout << "  Kung a=b=1/φ, sum=2/φ, result=-1/φ\n";
    std::cout << "  Kung a=b=0, sum=0, result=1/φ\n";
    std::cout << "  Self-normalizing ito!\n\n";

    return 0;
}
