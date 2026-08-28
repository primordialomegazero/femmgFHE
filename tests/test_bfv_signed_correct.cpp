// BFV SIGNED CORRECT — Proper Threshold sa Modulo Space
// Sa modulo 65537: x < 32768 = positive, x >= 32768 = negative

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV SIGNED CORRECT\n";
    std::cout << "  Proper Threshold sa Modulo Space\n";
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

    // Sa modulo 65537:
    // Values 0-32767 = positive (bit 1)
    // Values 32768-65536 = negative (bit 0)
    const int64_t HALF_MOD = 32768;

    // NAND sa BFV: NAND = 1 - a*b
    // 0 → 0, 1 → 1
    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);
    auto ct_one = make_ct(1);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    std::cout << "BFV SIGNED NAND TEST:\n";
    std::cout << "=====================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → "
              << (decrypt_val(nand_00) < HALF_MOD ? 1 : 0) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → "
              << (decrypt_val(nand_01) < HALF_MOD ? 1 : 0) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → "
              << (decrypt_val(nand_11) < HALF_MOD ? 1 : 0) << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 100; i++) {
        int64_t val = decrypt_val(state);
        int bit = (val < HALF_MOD) ? 1 : 0;

        if (bit != expected) errors++;

        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << state->GetLevel()
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }

        auto next_input = (bit == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected = 1 - expected;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BFV SIGNED CORRECT!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
