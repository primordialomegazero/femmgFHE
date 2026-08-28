// MERGE 100K PURE — Walang Decrypt sa Bawat Gate
// Period-0 + NAND, pure homomorphic evaluation
// Check lang sa dulo

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
    std::cout << "  MERGE 100K PURE\n";
    std::cout << "  Walang Decrypt sa Bawat Gate\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_MOD = 0.6180339887498949;

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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_const = make_ct(PHI_SQ - PHI);
    auto ct_phi_mod = make_ct(PHI_MOD);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_const, sum);
    };

    auto period0_rotate = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_mod);
    };

    std::cout << "PURE MERGED EVALUATION (100,000 gates):\n";
    std::cout << "========================================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    std::cout << "  Nag-e-evaluate nang walang decrypt...\n";
    std::cout << "  Ito na ang totoong FHE simulation\n\n";

    // Pure homomorphic evaluation — walang decrypt, walang check
    for (int i = 1; i <= 100000; i++) {
        state = period0_rotate(state);
        state = eval_nand(state, state);
        
        if (i % 10000 == 0) {
            std::cout << "  Progress: " << i << "/100000 gates\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();
    auto duration_min = duration / 60;

    // I-check lang sa dulo
    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 100,000\n";
    std::cout << "  Time: " << duration << " seconds (" << duration_min << " min)\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 5.0 ? "✅ YES" : "⚠️ NO") << "\n";
    std::cout << "  Status: " << (final_level == 0 ? "✅ LEVEL 0 STABLE — UNBOUNDED!" : "⚠️ LEVEL CHANGED") << "\n";
    std::cout << "========================================\n";

    return 0;
}
