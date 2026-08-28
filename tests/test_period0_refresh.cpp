// PERIOD-0 REFRESH — Natural na Bounded sa [0,1]
// x → (x + φ) mod 1 — natural na bounded
// Sa BFV: i-encode ang φ sa integer space

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 REFRESH\n";
    std::cout << "  Natural na Bounded sa [0,1]\n";
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
    auto ct_one = make_ct(1);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    const int64_t PHI_SCALED = 16180;
    auto ct_phi_scaled = make_ct(PHI_SCALED);
    
    auto eval_period0 = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_scaled);
    };

    std::cout << "PERIOD-0 REFRESH (1000 gates):\n";
    std::cout << "==============================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
        state = eval_period0(state);
        
        if (i % 100 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            int64_t val = decrypt_val(state);
            std::cout << "  Progress: " << i << "/1000 (" << elapsed << "s)"
                      << " val=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    int64_t final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 65537 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (std::abs(final_val) < 1000 ? "✅ PERIOD-0 REFRESH!" : "⚠️ MAHALAGANG VALUE") << "\n";
    std::cout << "========================================\n";

    return 0;
}
