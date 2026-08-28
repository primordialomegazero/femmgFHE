// ZERO CONVERGENCE — Natural Zero Threshold
// Addition only, walang multiplication
// Ang zero mismo ang separator

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
    std::cout << "  ZERO CONVERGENCE\n";
    std::cout << "  Addition Only, Natural Zero\n";
    std::cout << "========================================\n\n";

    const double PHI_MOD = 0.6180339887498949;

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
    auto ct_1 = make_ct(PHI_MOD);
    auto ct_one = make_ct(1.0);

    // NAND: 1 - (a+b) — addition lang
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    std::cout << "NAND TEST (addition only):\n";
    std::cout << "=========================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Ang susi: paano gawing zero ang NAND(1,1)?
    // NAND(1,1) = 1 - 2(0.618) = 1 - 1.236 = -0.236
    // Kung i-add natin ang φ, magiging 0.382
    // Kung i-subtract natin ang φ, magiging -0.854
    
    // Subukan: NAND + shift para ang (1,1) ay zero
    // -0.236 + 0.236 = 0
    // Kailangan natin ang eksaktong 0.236
    
    // Ang 0.236 = 1/φ³
    const double PHI_CUBE_INV = 1.0 / (1.6180339887498948482 * 1.6180339887498948482 * 1.6180339887498948482);
    
    auto ct_phi_cube_inv = make_ct(PHI_CUBE_INV);
    
    auto eval_nand_shifted = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand = cc->EvalSub(ct_one, sum);
        return cc->EvalAdd(nand, ct_phi_cube_inv);
    };

    std::cout << "SHIFTED NAND (para zero sa 1,1):\n";
    std::cout << "================================\n\n";

    auto nand_s_00 = eval_nand_shifted(ct_0, ct_0);
    auto nand_s_01 = eval_nand_shifted(ct_0, ct_1);
    auto nand_s_11 = eval_nand_shifted(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_s_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_s_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_s_11) << " (dapat 0)\n";
    std::cout << "  Level: " << nand_s_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "SHIFTED CHAIN (50 gates):\n";
    std::cout << "=========================\n\n";

    auto state = eval_nand_shifted(ct_1, ct_1);
    
    for (int i = 0; i < 50; i++) {
        state = eval_nand_shifted(state, state);
        
        if (i < 5 || i >= 45) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    return 0;
}
