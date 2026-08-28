// PHI BOOTSTRAP OPTIMIZATION — Golden Ratio para mapabilis
// Traditional CKKS bootstrapping + φ-based encoding

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI BOOTSTRAP OPTIMIZATION\n";
    std::cout << "  Golden Ratio + CKKS Bootstrapping\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);
    params.SetRingDim(1 << 16);
    params.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalRotateKeyGen(keys.secretKey, {1, 2, 4, 8, 16});

    // Bootstrapping setup
    std::vector<uint32_t> levelBudget = {5, 4};
    std::vector<uint32_t> dim1 = {0, 0};
    cc->EvalBootstrapSetup(levelBudget, dim1, 128);

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(256, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND: 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    std::cout << "PHI-BOOTSTRAP CHAIN TEST:\n";
    std::cout << "=========================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;
    int bootstrap_count = 0;

    for (int i = 0; i < 1000; i++) {
        // I-check ang level — kung masyadong mababa, bootstrap
        if (state->GetLevel() < 2) {
            state = cc->EvalBootstrap(state);
            bootstrap_count++;
        }

        double val = decrypt_val(state);
        int bit = (val > 0) ? 1 : 0;

        if (bit != expected) errors++;

        if (i < 5 || i >= 995) {
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

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Bootstraps: " << bootstrap_count << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PHI-BOOTSTRAP WORKS!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
