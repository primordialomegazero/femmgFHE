// CKKS NAND CHAIN — 1000 Gates Walang Bootstrapping
// Test kung level 0 rin sa CKKS

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS NAND CHAIN\n";
    std::cout << "  1000 Gates Walang Bootstrapping\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_1 = make_ct(1.0);
    auto ct_one = make_ct(1.0);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    std::cout << "CKKS NAND CHAIN:\n";
    std::cout << "================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
        
        if (i % 100 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            std::cout << "  Progress: " << i << "/1000 (" << elapsed << "s)"
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (final_level == 0 ? "✅ CKKS LEVEL 0!" : "⚠️ LEVEL DROP") << "\n";
    std::cout << "========================================\n";

    return 0;
}
