// NATURAL THRESHOLD LEVEL 0
// threshold(x) = x - (φ² - φ) = x - 1
// Subtraction lang, walang multiplication
// Natural zero crossing sa φ² - φ = 1

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
    std::cout << "  NATURAL THRESHOLD LEVEL 0\n";
    std::cout << "  x - (φ² - φ) = x - 1\n";
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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(PHI_SQ - PHI);  // = 1

    // NAND raw: 1 - (a+b)
    auto eval_nand_raw = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Natural threshold: x - 1, level 0
    auto eval_threshold = [&](auto x) {
        return cc->EvalSub(x, ct_one);
    };

    std::cout << "NATURAL THRESHOLD TEST:\n";
    std::cout << "=======================\n\n";

    // Test values: 0.382 (bit 0) at 1.618 (bit 1)
    auto test_bit0 = make_ct(PHI_INV * PHI_INV);  // 1/φ² = 0.382
    auto test_bit1 = make_ct(PHI);                 // φ = 1.618

    auto thresh_0 = eval_threshold(test_bit0);
    auto thresh_1 = eval_threshold(test_bit1);

    std::cout << "  threshold(0.382) = " << decrypt_val(thresh_0) 
              << " → " << (decrypt_val(thresh_0) > 0 ? 1 : 0) << "\n";
    std::cout << "  threshold(1.618) = " << decrypt_val(thresh_1)
              << " → " << (decrypt_val(thresh_1) > 0 ? 1 : 0) << "\n";
    std::cout << "  Level: " << thresh_0->GetLevel() << "\n\n";

    // Full NAND with threshold
    auto eval_nand_full = [&](auto a, auto b) {
        auto raw = eval_nand_raw(a, b);
        return eval_threshold(raw);
    };

    std::cout << "FULL NAND TEST:\n";
    std::cout << "===============\n\n";

    auto nand_00 = eval_nand_full(ct_0, ct_0);
    auto nand_01 = eval_nand_full(ct_0, ct_1);
    auto nand_11 = eval_nand_full(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → "
              << (decrypt_val(nand_00) > 0 ? 1 : 0) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → "
              << (decrypt_val(nand_01) > 0 ? 1 : 0) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → "
              << (decrypt_val(nand_11) > 0 ? 1 : 0) << " (expected 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test na walang decrypt sa bawat gate
    std::cout << "CHAIN TEST (100 gates, walang decrypt):\n";
    std::cout << "=======================================\n\n";

    auto state = eval_nand_full(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_nand_full(state, state);
        
        if (i < 5 || i >= 95) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 5.0 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
