// COMPLETE FHE CHAIN — NAND + EvalSin Sign + Feedback
// Ang buong pipeline, walang decrypt sa gitna

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
    std::cout << "  COMPLETE FHE CHAIN\n";
    std::cout << "  NAND + Sign + Feedback\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND: 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Homomorphic sign: EvalSin
    auto eval_sign = [&](auto x) {
        return cc->EvalSin(x, -4.0, 4.0, 5);
    };

    // Complete gate: NAND + Sign
    auto eval_complete_nand = [&](auto a, auto b) {
        auto nand_raw = eval_nand(a, b);
        auto sign = eval_sign(nand_raw);
        return sign;
    };

    std::cout << "COMPLETE NAND TEST:\n";
    std::cout << "===================\n\n";

    auto nand_00 = eval_complete_nand(ct_0, ct_0);
    auto nand_01 = eval_complete_nand(ct_0, ct_1);
    auto nand_11 = eval_complete_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → "
              << (decrypt_val(nand_00) > 0 ? "1" : "0") << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → "
              << (decrypt_val(nand_01) > 0 ? "1" : "0") << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → "
              << (decrypt_val(nand_11) > 0 ? "1" : "0") << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "CHAIN TEST (10 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_complete_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 10; i++) {
        state = eval_complete_nand(state, state);
        
        double val = decrypt_val(state);
        std::cout << "  Gate " << i << ": " << val
                  << " → " << (val > 0 ? "1" : "0")
                  << " level=" << state->GetLevel() << "\n";
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Time: " << duration << " seconds\n";

    return 0;
}
