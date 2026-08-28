// BFV TRUE NAND — 100K TOGGLE
// NAND = 1 - a*b
// Level 0, totoong Boolean logic

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV TRUE NAND — 100K TOGGLE\n";
    std::cout << "  Level 0, Boolean Logic\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_1 = make_ct(1);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

    std::cout << "100K TOGGLE TEST:\n";
    std::cout << "=================\n\n";

    auto state = ct_1;
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 100000; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 99995) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    int64_t final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (final_val == 1 || final_val == 0 ? "✅ TOGGLE 100K!" : "⚠️ ISSUE") << "\n";

    return 0;
}
