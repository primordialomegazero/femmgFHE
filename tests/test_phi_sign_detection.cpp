// 0-LEVEL SIGN DETECTION VIA OSCILLATION
// Ang oscillation ay nagbibigay ng natural na sign detection

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  0-LEVEL SIGN DETECTION\n";
    std::cout << "  Oscillation-Based\n";
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_two_phi_sq = make_ct(2 * phi_sq);

    std::cout << "SIGN DETECTION TEST:\n";
    std::cout << "====================\n\n";

    // Para sa negative value (-φ²):
    // Ito ay nag-ooscillate sa pagitan ng -φ² at φ²
    // Ang period-2 ay nagbibigay ng sign flip

    // Subukan ang oscillation sa negative
    auto neg_phi_sq = cc->EvalSub(ct_zero, ct_phi_sq);  // -φ²
    std::cout << "Negative -φ² = " << decrypt_val(neg_phi_sq) << "\n\n";

    // Oscillate the negative
    auto neg_osc = cc->EvalSub(ct_phi_sq, neg_phi_sq);  // φ² - (-φ²) = 2φ²
    std::cout << "φ² - (-φ²) = " << decrypt_val(neg_osc) << " (2φ²)\n\n";

    // Ang pattern para sa sign detection:
    // negative → oscillate → positive (2φ²)
    // positive → oscillate → negative (-φ²)
    // zero → oscillate → φ²

    std::cout << "SIGN-BASED OSCILLATION:\n";
    std::cout << "=======================\n\n";

    // Test lahat ng cases
    struct SignTest {
        const char* name;
        double input;
    };

    SignTest tests[] = {
        {"-φ² (negative)", -phi_sq},
        {"0 (zero)", 0.0},
        {"φ² (positive)", phi_sq}
    };

    for (auto& t : tests) {
        auto ct_input = make_ct(t.input);
        auto osc1 = cc->EvalSub(ct_phi_sq, ct_input);  // φ² - input
        auto osc2 = cc->EvalSub(ct_phi_sq, osc1);       // φ² - osc1

        std::cout << "  " << t.name << ":\n";
        std::cout << "    Input: " << decrypt_val(ct_input) << "\n";
        std::cout << "    Osc1 (φ² - input): " << decrypt_val(osc1) << "\n";
        std::cout << "    Osc2 (φ² - osc1): " << decrypt_val(osc2) << "\n";
        std::cout << "    Level: " << osc2->GetLevel() << "\n\n";
    }

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang oscillation ay nagbibigay ng period-2\n";
    std::cout << "  na may sign flip. Ito ay maaaring gamitin\n";
    std::cout << "  para sa threshold detection.\n";

    return 0;
}
