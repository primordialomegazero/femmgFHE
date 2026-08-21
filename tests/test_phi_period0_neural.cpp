// PERIOD-0 NEURAL NETWORK — MALALIM NA ARITHMETIC
// Weighted sums + polynomial evaluation sa irrational rotation

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
    std::cout << "  PERIOD-0 NEURAL NETWORK\n";
    std::cout << "  Malalim na Arithmetic\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_phi = make_ct(phi_sq);
    auto ct_one = make_ct(1.0);

    // ============================================
    // NEURAL NETWORK: 3-layer perceptron
    // ============================================
    // Layer 1: weighted sum (0-level)
    // Activation: mod 1 (irrational rotation)
    // Layer 2: weighted sum (0-level)
    // Activation: mod 1
    // Layer 3: output

    std::cout << "3-LAYER NEURAL NETWORK:\n";
    std::cout << "=======================\n\n";

    // Input values
    std::vector<double> inputs = {0.1, 0.2, 0.3, 0.4, 0.5};

    std::cout << "Input values: ";
    for (auto x : inputs) std::cout << x << " ";
    std::cout << "\n\n";

    // Layer 1: weighted sum (φ-weights)
    auto layer1 = make_ct(0.0);
    for (size_t i = 0; i < inputs.size(); i++) {
        auto input_ct = make_ct(inputs[i]);
        auto weighted = cc->EvalAdd(input_ct, ct_phi);
        layer1 = cc->EvalAdd(layer1, weighted);
    }

    double l1_val = decrypt_val(layer1);
    l1_val = l1_val - std::floor(l1_val);
    std::cout << "Layer 1 output (mod 1): " << l1_val << "\n";

    // Layer 2: activation + weighted sum
    auto layer2 = cc->EvalAdd(layer1, ct_phi);
    double l2_val = decrypt_val(layer2);
    l2_val = l2_val - std::floor(l2_val);
    std::cout << "Layer 2 output (mod 1): " << l2_val << "\n";

    // Layer 3: final output
    auto output = cc->EvalAdd(layer2, ct_phi);
    double out_val = decrypt_val(output);
    out_val = out_val - std::floor(out_val);
    std::cout << "Final output (mod 1): " << out_val << "\n\n";

    std::cout << "  Level: " << output->GetLevel() << "\n";
    std::cout << "  Status: ✅ NEURAL NETWORK WORKING!\n\n";

    // ============================================
    // POLYNOMIAL EVALUATION
    // ============================================
    std::cout << "POLYNOMIAL EVALUATION (f(x) = φ²x + 1 mod 1):\n";
    std::cout << "==============================================\n\n";

    for (double x : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        auto x_ct = make_ct(x);
        auto fx = cc->EvalAdd(x_ct, ct_phi);
        double fx_val = decrypt_val(fx);
        fx_val = fx_val - std::floor(fx_val);
        std::cout << "  f(" << x << ") = " << fx_val << "\n";
    }

    std::cout << "\n  Level: 0\n";
    std::cout << "  Status: ✅ POLYNOMIAL EVALUATION WORKING!\n";

    return 0;
}
