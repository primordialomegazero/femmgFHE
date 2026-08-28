// EMERGENT THRESHOLD CHAIN
// f(x) = φ² * x ang threshold
// NAND = φ² * (φ² - (a+b) - φ)
// Zero crossing ang natural na separation

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
    std::cout << "  EMERGENT THRESHOLD CHAIN\n";
    std::cout << "  f(x) = φ² * x\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_phi_inv = make_ct(PHI_INV);
    auto ct_0 = make_ct(0.0);

    // NAND: φ² - (a+b) - φ, tapos i-scale ng φ²
    auto eval_nand = [&](auto a, auto b) {
        // Raw NAND: φ² - (a+b) - φ = 1 - (a+b) dahil φ²-φ=1
        auto sum = cc->EvalAdd(a, b);
        auto raw = cc->EvalSub(make_ct(1.0), sum);
        
        // Scale ng φ² para sa natural threshold
        return cc->EvalMult(raw, ct_phi_sq);
    };

    std::cout << "EMERGENT NAND TEST:\n";
    std::cout << "===================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_phi_inv);
    auto nand_11 = eval_nand(ct_phi_inv, ct_phi_inv);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → " 
              << (decrypt_val(nand_00) > 0 ? 1 : 0) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → "
              << (decrypt_val(nand_01) > 0 ? 1 : 0) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → "
              << (decrypt_val(nand_11) > 0 ? 1 : 0) << "\n\n";

    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_phi_inv, ct_phi_inv);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_nand(state, state);
        
        if (i < 3 || i >= 97) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";

    return 0;
}
