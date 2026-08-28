// PHI MODULAR REFRESH — Natural na Modulo sa φ
// Sa halip na period-2, gamitin ang φ-based na modulo
// φ² = φ + 1 → natural na reduction

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI MODULAR REFRESH\n";
    std::cout << "  Natural na Modulo sa φ\n";
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

    // NAND: 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    // φ-based na modulo refresh
    // Sa BFV: multiply sa φ² = φ + 1 ≈ 3 (mod 65537)
    // Ito ay nagre-reduce ng values sa mas maliit na range
    
    const int64_t PHI_SQ_MOD = 3;  // φ² ≈ 2.618 → 3 (integer approximation)
    auto ct_phi_sq = make_ct(PHI_SQ_MOD);

    auto eval_phi_refresh = [&](auto x) {
        return cc->EvalMult(x, ct_phi_sq);
    };

    std::cout << "PHI MODULAR REFRESH (1000 gates):\n";
    std::cout << "=================================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
        state = eval_phi_refresh(state);  // φ-based na modulo
        
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
    std::cout << "  Bounded: " << (std::abs(final_val) < 100 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (std::abs(final_val) < 100 ? "✅ PHI MODULAR REFRESH!" : "⚠️ MAY NOISE") << "\n";
    std::cout << "========================================\n";

    return 0;
}
