// NATURAL MIDPOINT THRESHOLD
// φ - 1/φ = 1 — natural separator
// Bit 0 → 1/φ² = 0.382
// Bit 1 → φ = 1.618
// Midpoint: 1.0

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
    std::cout << "  NATURAL MIDPOINT THRESHOLD\n";
    std::cout << "  φ - 1/φ = 1\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV_SQ = PHI_INV * PHI_INV;

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
    auto ct_const = make_ct(PHI_SQ - PHI);  // = 1
    auto ct_phi_mod = make_ct(PHI_INV);  // period-0 step

    // NAND: φ² - (a+b) - φ = 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_const, sum);
    };

    // Period-0: x + 1/φ
    auto period0 = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_mod);
    };

    std::cout << "NATURAL SEPARATION TEST:\n";
    std::cout << "=======================\n\n";
    std::cout << "  Bit 0 → " << PHI_INV_SQ << " (1/φ²)\n";
    std::cout << "  Bit 1 → " << PHI << " (φ)\n";
    std::cout << "  Midpoint → " << 1.0 << " (φ - 1/φ)\n\n";

    // Test: self-normalizing sequence
    std::cout << "SELF-NORMALIZING SEQUENCE:\n";
    std::cout << "==========================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    std::cout << "  Initial: " << decrypt_val(state) << "\n";

    for (int i = 0; i < 10; i++) {
        state = period0(state);
        state = eval_nand(state, state);
        double val = decrypt_val(state);
        std::cout << "  Step " << i << ": " << val << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Kung ang values ay nag-o-oscillate sa pagitan\n";
    std::cout << "  ng 0.382 at 1.618, may natural tayong separation\n";
    std::cout << "  na hindi kailangan ng external threshold\n";

    return 0;
}
