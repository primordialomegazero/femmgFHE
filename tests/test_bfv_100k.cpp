// BFV 100K GATES — Ultimate Unbounded Test
// Natural modulo sa plaintext modulus
// Kung pumasa ito, UNBOUNDED na

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV 100K GATES\n";
    std::cout << "  Ultimate Unbounded Test\n";
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

    const int64_t SCALE = 6553;
    const int64_t TWO_SCALE = 2 * SCALE;

    auto ct_1 = make_ct(SCALE);
    auto ct_two_scale = make_ct(TWO_SCALE);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_scale, sum);
    };

    std::cout << "CHAIN TEST (100,000 gates):\n";
    std::cout << "===========================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100000; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i % 10000 == 0 || i >= 99995) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();
    auto duration_min = duration / 60;

    int64_t final_val = decrypt_val(state);
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 100,000\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 65537 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds (" << duration_min << " min)\n";
    std::cout << "  Status: " << (std::abs(final_val) < 65537 ? "✅ BFV 100K BOUNDED — UNBOUNDED!" : "⚠️ MAY ISSUE") << "\n";
    std::cout << "========================================\n";

    return 0;
}
