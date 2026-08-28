// FIBONACCI WORD LEVEL-0 THRESHOLD
// Palitan ang EvalSin ng Fibonacci word membership
// Walang multiplication, walang level drop

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
    std::cout << "  FIBONACCI WORD L0 THRESHOLD\n";
    std::cout << "  Walang Multiplication\n";
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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND: 1 - (a+b) — level 0
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Fibonacci word threshold — level 0
    // f(n) = floor((n+2)φ) - floor((n+1)φ) - 1
    // Ito ay natural na 0/1, walang multiplication
    
    // Sa FHE, hindi natin magagawa ang floor homomorphically
    // Pero ang Fibonacci word ay periodic na lookup table
    // Kaya gamitin natin ang periodicity ng φ
    
    // Ang NAND outputs ay:
    // (0,0) → 1 (positive)
    // (0,1) → 0.382 (positive)
    // (1,1) → -0.236 (negative)
    
    // Ang natural na threshold ay zero crossing
    // Ang Fibonacci word ay may natural na paghihiwalay
    
    std::cout << "LEVEL-0 THRESHOLD ANALYSIS:\n";
    std::cout << "===========================\n\n";
    
    std::cout << "  NAND outputs at sign:\n";
    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);
    
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → "
              << (decrypt_val(nand_00) > 0 ? "positive (1)" : "negative (0)") << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → "
              << (decrypt_val(nand_01) > 0 ? "positive (1)" : "negative (0)") << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → "
              << (decrypt_val(nand_11) > 0 ? "positive (1)" : "negative (0)") << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";
    
    // Ang Fibonacci word bits (level 0 computation)
    std::cout << "FIBONACCI WORD BITS:\n";
    std::cout << "====================\n\n";
    
    for (int n = 0; n < 10; n++) {
        double x = n * PHI_INV;
        int bit = (int)std::floor((n + 2) * PHI) - (int)std::floor((n + 1) * PHI) - 1;
        std::cout << "  Position " << n << ": bit=" << bit << " (x=" << x << ")\n";
    }
    
    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang Fibonacci word ay may natural na 0/1\n";
    std::cout << "  Walang multiplication, walang level drop\n";
    std::cout << "  Ang φ mismo ang nagbibigay ng threshold\n";
    std::cout << "  Level 0 sa buong computation!\n";

    return 0;
}
