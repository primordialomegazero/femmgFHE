// FIBONACCI WORD FHE — Natural Homomorphic Threshold
// Tamang CKKS setup

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI WORD FHE\n";
    std::cout << "  Natural Homomorphic Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

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

    // Fibonacci word bits
    auto fib_word_bit = [&](int n) {
        int val = (int)std::floor((n + 2) * PHI) - (int)std::floor((n + 1) * PHI);
        return val - 1;
    };

    std::cout << "FIBONACCI WORD BITS:\n";
    std::cout << "===================\n\n";
    for (int n = 1; n <= 20; n++) {
        std::cout << "  " << n << " → " << fib_word_bit(n) << "\n";
    }

    // NAND sa φ-space
    const double PHI_INV = 1.0 / PHI;
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    std::cout << "\nNAND OUTPUTS:\n";
    std::cout << "=============\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Ang Fibonacci word ang natural na threshold
    // NAND(0,0) = 1 → Fibonacci word position na may 1
    // NAND(1,1) = -0.236 → Fibonacci word position na may 0
    
    // I-observe ang natural na paghihiwalay
    std::cout << "THRESHOLD OBSERVATION:\n";
    std::cout << "======================\n\n";
    std::cout << "  NAND(0,0) = 1 → bit 1\n";
    std::cout << "  NAND(0,1) = 0.382 → bit 1\n";
    std::cout << "  NAND(1,1) = -0.236 → bit 0\n\n";
    std::cout << "  Ang zero crossing ay nasa pagitan ng 0.382 at -0.236\n";
    std::cout << "  Fibonacci word ay may natural na paghihiwalay\n";

    return 0;
}
