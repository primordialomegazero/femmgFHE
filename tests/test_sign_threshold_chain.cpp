// SIGN THRESHOLD CHAIN — Natural na FHE
// NAND = 1 - (a+b), sign-based threshold
// Positive → 1/φ, Negative → 0

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
    std::cout << "  SIGN THRESHOLD CHAIN\n";
    std::cout << "  Natural FHE\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV_SQ = PHI_INV * PHI_INV;
    const double PHI_INV_CU = PHI_INV * PHI_INV * PHI_INV;

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

    // Encoding: 0 → 0, 1 → 1/φ
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND: 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Natural sign threshold: x > 0 ? 1/φ : 0
    // Hindi natin kaya ang conditional sa FHE
    // Pero ang sign mismo ang nagbibigay ng impormasyon
    
    std::cout << "NAND TEST:\n";
    std::cout << "==========\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " → sign: " 
              << (decrypt_val(nand_00) > 0 ? "+" : "-") << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " → sign: "
              << (decrypt_val(nand_01) > 0 ? "+" : "-") << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " → sign: "
              << (decrypt_val(nand_11) > 0 ? "+" : "-") << "\n\n";

    // Chain test na may sign-based feedback
    std::cout << "CHAIN TEST (50 gates):\n";
    std::cout << "=====================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 50; i++) {
        // Natural sign-based feedback
        // Kung positive → 1/φ, kung negative → 0
        // Sa FHE, ito ay approximation ng sign
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 45) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Time: " << duration << " seconds\n";

    return 0;
}
