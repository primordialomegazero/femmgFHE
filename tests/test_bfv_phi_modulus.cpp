// BFV φ-MODULUS — Natural Periodicity
// I-set ang plaintext modulus para φ-based
// Natural na modulo sa φ encoding

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
    std::cout << "  BFV φ-MODULUS\n";
    std::cout << "  Natural Periodicity\n";
    std::cout << "========================================\n\n";

    // φ-based values
    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    
    // Sa integers: 1 → 1, 0 → 0
    // NAND: 2 - (a+b)
    // Values: 0, 1, 2
    // Natural modulus: 3 para sa bounded {0,1,2}
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(3);  // Natural modulus para sa {0,1,2}
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    std::cout << "PLAINTEXT MODULUS: 3\n";
    std::cout << "===================\n\n";
    std::cout << "  Natural values: 0, 1, 2\n";
    std::cout << "  Modulo 3: 0→0, 1→1, 2→2, 3→0, 4→1, 5→2\n\n";

    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);
    auto ct_2 = make_ct(2);

    // NAND: 2 - (a+b) mod 3
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_2, sum);
    };

    std::cout << "NAND TEST (mod 3):\n";
    std::cout << "==================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected 2)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 95) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    int64_t final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Bounded: " << (final_val >= 0 && final_val <= 2 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
