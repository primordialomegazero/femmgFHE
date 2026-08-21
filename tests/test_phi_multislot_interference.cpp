// MULTI-SLOT INTERFERENCE XOR — 0-LEVEL
// Dalawang slots na may magkaibang oscillation
// Ang interference pattern ay nagbibigay ng unique signature

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTI-SLOT INTERFERENCE XOR\n";
    std::cout << "  0-Level Attempt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

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

    std::cout << "MULTI-SLOT SETUP:\n";
    std::cout << "=================\n\n";
    std::cout << "  Slot 0: φ²-oscillator (period 2)\n";
    std::cout << "  Slot 1: φ³-oscillator (period 2, ibang scale)\n\n";

    // Para sa sum=0:
    auto sum0_slot0 = make_ct(0.0);
    auto sum0_slot1 = make_ct(0.0);

    // Para sa sum=φ²:
    auto sum1_slot0 = make_ct(phi_sq);
    auto sum1_slot1 = make_ct(phi_sq);

    // Para sa sum=2φ²:
    auto sum2_slot0 = make_ct(2*phi_sq);
    auto sum2_slot1 = make_ct(2*phi_sq);

    std::cout << "INTERFERENCE TEST (3 iterations):\n";
    std::cout << "=================================\n\n";

    // I-oscillate ang bawat slot ng iba't ibang factor
    // Slot 0: φ² - x
    // Slot 1: φ³ - x

    auto osc_slot0 = [&](auto x) { return cc->EvalSub(make_ct(phi_sq), x); };
    auto osc_slot1 = [&](auto x) { return cc->EvalSub(make_ct(phi_cu), x); };

    for (int case_num = 0; case_num < 3; case_num++) {
        auto s0 = (case_num == 0) ? sum0_slot0 : (case_num == 1) ? sum1_slot0 : sum2_slot0;
        auto s1 = (case_num == 0) ? sum0_slot1 : (case_num == 1) ? sum1_slot1 : sum2_slot1;

        std::cout << "  Case " << case_num << " (sum=" << case_num << "φ²):\n";

        for (int i = 0; i < 3; i++) {
            s0 = osc_slot0(s0);
            s1 = osc_slot1(s1);

            double v0 = decrypt_val(s0);
            double v1 = decrypt_val(s1);
            double interference = std::abs(v0 - v1);

            std::cout << "    Iter " << i << ": slot0=" << v0
                      << " slot1=" << v1
                      << " interference=" << interference
                      << " level=" << s0->GetLevel() << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang interference pattern ay nagbibigay\n";
    std::cout << "  ng unique signature para sa bawat case.\n";
    std::cout << "  Kung ito ay stable, maaaring gamitin\n";
    std::cout << "  para sa 0-level XOR threshold.\n";

    return 0;
}
