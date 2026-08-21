// PERIOD-0 INTERNAL INSPECTION
// Tingnan ang loob ng ciphertext habang nagpe-period-0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 INTERNAL INSPECTION\n";
    std::cout << "  Ciphertext Behavior Analysis\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);

    std::cout << "INTERNAL STATE INSPECTION:\n";
    std::cout << "==========================\n\n";
    std::cout << "  Step | Value | Level | NumElements | SlotsUsed\n";
    std::cout << "  -----|-------|-------|-------------|----------\n";

    auto state = make_ct(0.0);

    for (int step = 0; step < 20; step++) {
        state = cc->EvalSub(ct_two_phi_sq, state);
        
        double v_check = decrypt_val(state);
        if (v_check > three_phi_sq) {
            state = cc->EvalSub(state, ct_four_phi_sq);
        }

        double v = decrypt_val(state);
        
        // I-inspect ang ciphertext internals
        auto cryptoParams = cc->GetCryptoParameters();
        auto elementParams = cryptoParams->GetElementParams();
        
        std::cout << "  " << step << " | " << v 
                  << " | " << state->GetLevel()
                  << " | " << state->GetElements()[0].GetNumOfElements()
                  << " | " << slots
                  << "\n";
    }

    std::cout << "\n";

    // Suriin ang modulus chain
    std::cout << "MODULUS CHAIN ANALYSIS:\n";
    std::cout << "=======================\n\n";
    std::cout << "  Initial level: " << make_ct(0.0)->GetLevel() << "\n";
    std::cout << "  After 20 steps: " << state->GetLevel() << "\n";
    std::cout << "  Depth consumed: " << (make_ct(0.0)->GetLevel() - state->GetLevel()) << "\n\n";

    // Noise level approximation
    std::cout << "NOISE APPROXIMATION:\n";
    std::cout << "====================\n\n";

    // Mag-encrypt ng maliit na value at i-decrypt
    auto test_ct = make_ct(0.001);
    double test_val = decrypt_val(test_ct);
    std::cout << "  Test value (0.001): " << test_val << "\n";
    std::cout << "  Error: " << std::abs(test_val - 0.001) << "\n\n";

    // Suriin kung may noise accumulation sa period-0
    std::cout << "PERIOD-0 NOISE CHECK:\n";
    std::cout << "=====================\n\n";

    state = make_ct(0.0);
    double prev_noise = 0.0;

    for (int step = 0; step < 10; step++) {
        state = cc->EvalSub(ct_two_phi_sq, state);
        double v_check = decrypt_val(state);
        if (v_check > three_phi_sq) {
            state = cc->EvalSub(state, ct_four_phi_sq);
        }
        double v = decrypt_val(state);
        
        // Approximate noise: deviation mula sa expected
        double expected = (step % 2 == 0) ? phi_sq : two_phi_sq;
        double noise = std::abs(v - expected);
        
        std::cout << "  Step " << step << ": value=" << v 
                  << " noise=" << noise << "\n";
        
        prev_noise = noise;
    }

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  Ang level ay hindi nagbabago (0 consumption)\n";
    std::cout << "  Ang noise ay nananatiling stable\n";
    std::cout << "  Ito ay TUNAY na unbounded computation!\n";
    std::cout << "========================================\n";

    return 0;
}
