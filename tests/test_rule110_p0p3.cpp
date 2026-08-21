// RULE 110 + PERIOD-0×3 COMBO
// x_{n+1} = K_i - (x_n + φ_mod) para sa bawat cell
// Sa halip na neighbor sum, gamitin ang bounded cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + PERIOD-0×3\n";
    std::cout << "  Bounded Cell Cycle\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double phi_mod = phi_sq - 2.0;

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

    // K values para sa period-3 cycle
    auto K0 = make_uniform(phi_sq);       // φ²
    auto K1 = make_uniform(two_phi_sq);   // 2φ²
    auto K2 = make_uniform(three_phi_sq); // 3φ²
    auto phi_mod_ct = make_uniform(phi_mod);

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 + P0×3 (100 steps):\n";
    std::cout << "============================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Kunin ang tamang K para sa step
        auto K_use = (step % 3 == 0) ? K0 : (step % 3 == 1) ? K1 : K2;

        // Bounded cycle: x = K - (x + φ_mod)
        auto shifted = cc->EvalAdd(state, phi_mod_ct);
        state = cc->EvalSub(K_use, shifted);

        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) < 5 * phi_sq);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ P0×3 BOUNDED!" : "❌") << "\n";

    return 0;
}
