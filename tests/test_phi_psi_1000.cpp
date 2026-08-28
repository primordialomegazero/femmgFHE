// PHI-PSI 1000 — Walang Decrypt sa Gitna
// Pure homomorphic evaluation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI-PSI 1000\n";
    std::cout << "  Walang Decrypt sa Gitna\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_phi = make_ct(PHI);
    auto ct_one = make_ct(1.0);

    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_one, x);
    };

    std::cout << "PURE EVALUATION (1000 gates):\n";
    std::cout << "=============================\n\n";

    auto state = ct_phi;
    auto start = high_resolution_clock::now();

    // Walang decrypt sa gitna!
    for (int i = 0; i < 1000; i++) {
        state = eval_not(state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    // I-check lang sa dulo
    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val - PHI) < 0.01 || std::abs(final_val - PSI) < 0.01 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (final_level == 0 ? "✅ LEVEL 0 — PURE FHE!" : "⚠️") << "\n";

    return 0;
}
