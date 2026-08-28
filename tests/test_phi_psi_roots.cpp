// φ-ψ ROOTS — Natural Binary States
// φ at ψ ang natural na states
// Walang threshold — ang roots mismo ang sagot

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-ψ ROOTS — Natural Binary\n";
    std::cout << "  Walang Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

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

    auto ct_psi = make_ct(PSI);
    auto ct_phi = make_ct(PHI);

    // Natural NAND: φ + ψ - (a+b)
    // (φ,φ) → φ+ψ-2φ = ψ - φ = -2.236... hindi tamang encoding
    // Kaya subukan natin: NAND = φ + ψ - (a+b)
    // (φ,φ) → 1 - 3.236 = -2.236
    // (φ,ψ) → 1 - 1 = 0
    // (ψ,ψ) → 1 + 1.236 = 2.236

    // Hmm... hindi ito gumagana sa direktang addition

    // Subukan ang multiplication: NAND = φ * ψ - a * b
    // (φ,φ) → φψ - φ² = -1 - 2.618 = -3.618
    // Hindi rin...

    // Ang natural na property: φ * ψ = -1
    // at φ + ψ = 1
    
    // NAND(a,b) = φ + ψ - a - b + φψ
    // (φ,φ) → 1 - 3.236 - 1 = -3.236
    // (φ,ψ) → 1 - 1 - 1 = -1
    // (ψ,ψ) → 1 + 1.236 - 1 = 1.236

    // Ang pinaka-natural: NAND = φψ - ab
    // (φ,φ) → -1 - φ² = -3.618
    // (φ,ψ) → -1 - (-1) = 0
    // (ψ,ψ) → -1 - ψ² = -1 - 0.382 = -1.382

    std::cout << "NATURAL φ-ψ PROPERTIES:\n";
    std::cout << "=======================\n\n";
    std::cout << "  φ + ψ = " << (PHI + PSI) << "\n";
    std::cout << "  φ * ψ = " << (PHI * PSI) << "\n";
    std::cout << "  φ² = " << (PHI * PHI) << "\n";
    std::cout << "  ψ² = " << (PSI * PSI) << "\n\n";

    // Ang pinaka-simpleng approach: NAND = 1 - (a+b)
    // Sa φ-ψ encoding: 0 → ψ, 1 → φ
    // 1 - (φ+φ) = 1 - 2φ = -2.236 (dapat ψ)
    // 1 - (φ+ψ) = 1 - 1 = 0 (dapat φ)
    // 1 - (ψ+ψ) = 1 - 2ψ = 2.236 (dapat φ)

    // I-multiply ang encoding para ma-normalize
    // Kung 0 → ψ, 1 → φ:
    // (0,0) → 1 - 2ψ = 1 + 1.236 = 2.236
    // (0,1) → 1 - (φ+ψ) = 0
    // (1,1) → 1 - 2φ = 1 - 3.236 = -2.236

    // NAND(0,0) = 2.236 → dapat 1 (φ)
    // NAND(0,1) = 0 → dapat 1 (φ)
    // NAND(1,1) = -2.236 → dapat 0 (ψ)

    std::cout << "SIMPLE NAND: 1 - (a+b)\n";
    std::cout << "========================\n\n";
    std::cout << "  NAND(ψ,ψ) = " << (1.0 - 2*PSI) << "\n";
    std::cout << "  NAND(ψ,φ) = " << (1.0 - (PHI+PSI)) << "\n";
    std::cout << "  NAND(φ,φ) = " << (1.0 - 2*PHI) << "\n\n";

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  May natural na zero crossing sa (ψ,φ)\n";
    std::cout << "  Positive sa (ψ,ψ)\n";
    std::cout << "  Negative sa (φ,φ)\n";

    return 0;
}
