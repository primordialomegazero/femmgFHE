// PHI NATURAL ITERATION — Ang φ mismo ang nagko-compute
// x_{n+1} = φ - x_n
// Natural na computation, walang NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI NATURAL ITERATION\n";
    std::cout << "  Ang φ mismo ang nagko-compute\n";
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

    // Natural iteration: x → φ - x
    auto eval_iterate = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    std::cout << "NATURAL ITERATION TEST:\n";
    std::cout << "======================\n\n";

    // Simula sa 0
    auto state = make_ct(0.0);
    
    std::cout << "  Start: 0\n";
    
    for (int i = 0; i < 10; i++) {
        state = eval_iterate(state);
        double val = decrypt_val(state);
        std::cout << "  Step " << i << ": " << val << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  x → φ - x ay may pattern:\n";
    std::cout << "  0 → φ → 0 → φ → 0 → φ → ...\n";
    std::cout << "  Ito ay natural na toggle!\n";
    std::cout << "  Bounded sa [0, φ]\n\n";

    // Chain test — 1000 gates
    std::cout << "CHAIN TEST (1000 gates):\n";
    std::cout << "========================\n\n";

    state = make_ct(0.0);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_iterate(state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅" : "⚠️") << "\n";

    return 0;
}
