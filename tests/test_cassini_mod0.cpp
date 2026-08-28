// CASSINI + MODULO 0 — Natural Threshold
// Cassini: F(n-1)F(n+1) - F(n)² = (-1)^n
// Modulo 0: x mod 0 = undefined, pero x - x = 0
// Ang zero mismo ay natural na separator

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
    std::cout << "  CASSINI + MODULO 0\n";
    std::cout << "  Natural Zero Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    // Fibonacci numbers para sa Cassini
    // F(0)=0, F(1)=1, F(2)=1, F(3)=2, F(4)=3, F(5)=5
    
    // Cassini: F(n-1)F(n+1) - F(n)² = (-1)^n
    // Ito ay natural na ±1 oscillation
    
    // Subukan: NAND = Cassini-based
    // NAND(a,b) = (1 - (a+b)) * (1 - (a+b) - 1)
    // = (1 - (a+b)) * (-(a+b))
    
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_MOD);

    // NAND: 1 - (a+b) na may Cassini twist
    auto eval_nand_cassini = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand = cc->EvalSub(make_ct(1.0), sum);
        
        // Cassini: multiply ng (1 - sum) para sa natural ±1
        auto one_minus_sum = nand;
        auto cassini_factor = cc->EvalSub(make_ct(1.0), sum);
        
        // Ito ay magbibigay ng natural na zero crossing
        return cc->EvalMult(one_minus_sum, cassini_factor);
    };

    std::cout << "CASSINI NAND TEST:\n";
    std::cout << "=================\n\n";

    auto nand_00 = eval_nand_cassini(ct_0, ct_0);
    auto nand_01 = eval_nand_cassini(ct_0, ct_1);
    auto nand_11 = eval_nand_cassini(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Modulo 0: x - x = 0
    // Ang zero mismo ay natural na separator
    std::cout << "MODULO 0 TEST:\n";
    std::cout << "==============\n\n";

    auto zero_test = cc->EvalSub(nand_11, nand_11);
    std::cout << "  x - x = " << decrypt_val(zero_test) << " (dapat 0)\n\n";

    // Chain test
    std::cout << "CASSINI CHAIN (20 gates):\n";
    std::cout << "=========================\n\n";

    auto state = eval_nand_cassini(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        state = eval_nand_cassini(state, state);
        
        if (i < 5 || i >= 15) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Time: " << duration << " seconds\n";

    return 0;
}
