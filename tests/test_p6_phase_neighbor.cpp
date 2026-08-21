// PERIOD-6 PHASE ENCODING
// Ang neighbor interaction ay phase shift,
// hindi sum — para walang growth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-6 PHASE ENCODING\n";
    std::cout << "  Neighbor = Phase Shift\n";
    std::cout << "  Walang Growth!\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double K = phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // Period-6 states: 0, K, 2K, 2K, K, 0
    // Ang phase ay naka-encode sa state value
    // Hindi sa sum ng neighbors

    // Initial: period-6 pattern sa slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        int state = i % 6;
        if (state == 0) init[i] = {0.0, 0.0};
        else if (state == 1) init[i] = {K, 0.0};
        else if (state == 2 || state == 3) init[i] = {2*K, 0.0};
        else init[i] = {K, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "PERIOD-6 PHASE (1000 steps):\n";
    std::cout << "============================\n\n";

    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Phase shift: i-rotate lang, hindi i-sum
        state = cc->EvalAtIndex(state, 1);
        
        // Period-6 transition: x -> 2K - x
        // (bounded map na walang neighbor sum)
        auto ct_twoK = make_uniform(2 * K);
        state = cc->EvalSub(ct_twoK, state);

        double v = decrypt_slot(state, 128);
        bool bounded = (v >= 0.0 && v <= 2 * K + 0.1);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PHASE BOUNDED!" : "❌") << "\n";

    return 0;
}
