// PHI REFRESH BFV — Natural Noise Reset
// Multiply sa φ para ma-reset ang values sa bounded range

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI REFRESH BFV\n";
    std::cout << "  Natural Noise Reset\n";
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
    auto ct_one = make_ct(1);
    auto ct_phi_mod = make_ct(40503);  // φ² = 2.618 * 10000 mod 65537

    // NAND: 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    // φ refresh: multiply sa φ mod 65537
    auto eval_refresh = [&](auto x) {
        return cc->EvalMult(x, ct_phi_mod);
    };

    std::cout << "BFV PHI REFRESH CHAIN (100 gates):\n";
    std::cout << "==================================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 100; i++) {
        state = eval_nand(state, state);
        state = eval_refresh(state);
        
        int64_t val = decrypt_val(state);
        int bit = (val == 1 || val == 0) ? val : (val < 32768 ? 1 : 0);

        if (bit != expected) errors++;

        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << state->GetLevel()
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }

        expected = 1 - expected;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PHI REFRESH WORKS!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
