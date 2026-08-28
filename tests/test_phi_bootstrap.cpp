// PHI BOOTSTRAP — Golden Ratio para sa Bootstrapping
// Sa halip na traditional bootstrapping, gamitin ang φ
// para natural na i-reset ang noise

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI BOOTSTRAP\n";
    std::cout << "  Golden Ratio Bootstrapping\n";
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

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    const int64_t PHI_SQ_SCALED = 26180;
    auto ct_phi_sq = make_ct(PHI_SQ_SCALED);

    auto eval_phi_bootstrap = [&](auto x) {
        return cc->EvalMult(x, ct_phi_sq);
    };

    std::cout << "PHI BOOTSTRAP CHAIN (1000 gates):\n";
    std::cout << "=================================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
        state = eval_phi_bootstrap(state);
        
        int64_t val = decrypt_val(state);
        int bit = (val == 1) ? 1 : 0;
        
        if (bit != expected) errors++;

        if (i < 5 || i >= 995) {
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

    int64_t final_val = decrypt_val(state);

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PHI BOOTSTRAP!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
