// BFV NAND — Exact Integer Arithmetic
// BFV ay exact, walang approximation error
// Baka mas natural ang threshold dito

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
    std::cout << "  BFV NAND — Exact Integer Arithmetic\n";
    std::cout << "  Natural Threshold Test\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(65537);
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

    // BFV exact integers
    // 0 → 0, 1 → 1
    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);
    auto ct_2 = make_ct(2);

    // NAND: 2 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_2, sum);
    };

    std::cout << "BFV NAND TEST:\n";
    std::cout << "==============\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // BFV ay exact — walang approximation error
    // Subukan ang chain
    std::cout << "BFV CHAIN (100 gates):\n";
    std::cout << "=====================\n\n";

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
    std::cout << "  Bounded: " << (std::abs(final_val) < 10 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
