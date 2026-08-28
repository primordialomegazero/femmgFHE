// CKKS MODULUS THRESHOLD — Natural Scaling
// Ang CKKS ay may natural na modulo sa scaling
// Subukan kung ang scaling mismo ang mag-threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS MODULUS THRESHOLD\n";
    std::cout << "  Natural Scaling\n";
    std::cout << "========================================\n\n";

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

    // I-scale nang malaki para makita ang natural na modulo
    // -0.236 * 1000 = -236
    // 0.382 * 1000 = 382
    auto test_neg = make_ct(-236.0);
    auto test_pos = make_ct(382.0);

    std::cout << "SCALED VALUES:\n";
    std::cout << "==============\n\n";
    std::cout << "  Negative: -236\n";
    std::cout << "  Positive: 382\n\n";

    // Sa malaking scale, ang zero crossing ay mas malinaw
    // Subukan ang NAND chain sa scaled space
    
    // NAND = 1000 * (φ² - (a+b) - φ)
    // = 1000 * (1 - (a+b))
    const double SCALE = 1000.0;
    
    auto eval_nand_scaled = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto result = cc->EvalSub(make_ct(SCALE), sum);
        return result;
    };

    std::cout << "SCALED NAND TEST:\n";
    std::cout << "=================\n\n";

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(618.0);  // 1000/φ

    auto nand_00 = eval_nand_scaled(ct_0, ct_0);
    auto nand_01 = eval_nand_scaled(ct_0, ct_1);
    auto nand_11 = eval_nand_scaled(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "SCALED CHAIN (20 gates):\n";
    std::cout << "========================\n\n";

    auto state = eval_nand_scaled(ct_1, ct_1);
    
    for (int i = 0; i < 20; i++) {
        state = eval_nand_scaled(state, state);
        double val = decrypt_val(state);
        
        if (i < 5 || i >= 15) {
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    return 0;
}
