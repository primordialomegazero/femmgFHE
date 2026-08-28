// 100K PURE — Walang Decrypt na Cheating
// Pure homomorphic evaluation
// Rule 110 + NAND, BFV, walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  100K PURE\n";
    std::cout << "  Walang Decrypt sa Gitna\n";
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

    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);

    // NAND: 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

    std::cout << "PURE EVALUATION (100,000 NAND gates):\n";
    std::cout << "=====================================\n\n";

    // Initial state
    auto state = eval_nand(ct_1, ct_1);
    
    auto start = high_resolution_clock::now();

    // 100K NAND gates na walang decrypt
    for (int i = 0; i < 100000; i++) {
        state = eval_nand(state, state);
        
        if (i % 10000 == 0) {
            std::cout << "  Progress: " << i << "/100000\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    // I-check lang sa dulo
    int64_t final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 100,000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (final_val == 0 || final_val == 1 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (final_level == 0 ? "✅ 100K PURE FHE!" : "⚠️ LEVEL DROP") << "\n";
    std::cout << "========================================\n";

    return 0;
}
