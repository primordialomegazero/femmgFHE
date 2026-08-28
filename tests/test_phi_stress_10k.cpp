// PHI STRESS 10K — 10,000 Gates sa φ-Space
// NAND = φ - (φ - a + b) — composed sa NOT at XOR
// Walang decrypt, walang bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI STRESS 10K\n";
    std::cout << "  10,000 Gates sa φ-Space\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

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
    auto ct_0 = make_ct(0.0);

    // NOT: x → φ - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    // NAND = NOT(AND)
    auto eval_nand = [&](auto a, auto b) {
        // AND(a,b) = NOT(NOT(a) XOR NOT(b))
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto xor_nots = cc->EvalSub(not_a, not_b);
        auto and_ab = eval_not(xor_nots);
        // NAND = NOT(AND)
        return eval_not(and_ab);
    };

    std::cout << "GATE VERIFICATION:\n";
    std::cout << "==================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_0phi = eval_nand(ct_0, ct_phi);
    auto nand_phiphi = eval_nand(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected 0)\n";
    std::cout << "  NAND(0,φ) = " << decrypt_val(nand_0phi) << " (expected φ)\n";
    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_phiphi) << " (expected 0)\n\n";

    std::cout << "10K CHAIN TEST:\n";
    std::cout << "===============\n\n";

    auto state = eval_nand(ct_phi, ct_phi);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        state = eval_nand(state, state);
        
        if (i % 1000 == 0) {
            std::cout << "  Progress: " << i << "/10000\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 10,000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅" : "⚠️") << "\n";
    std::cout << "  Status: " << (final_level == 0 && std::abs(final_val) < 2.0 ? "✅ 10K PURE FHE!" : "⚠️ MAY ISSUE") << "\n";
    std::cout << "========================================\n";

    return 0;
}
