// NATURAL QUANTUM — φ + ψ = 0 CANCELLATION
// H(x) = x + φ_mod, P(x) = x + ψ_mod
// Ang alternating sum ay ZERO — natural bounded!

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
    std::cout << "  NATURAL QUANTUM — φ+ψ=0\n";
    std::cout << "  Natural Cancellation\n";
    std::cout << "========================================\n\n";

    const double phi_mod = 0.6180339887498949;   // φ² mod 1
    const double psi_mod = -0.6180339887498949;  // ψ = -1/φ

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

    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_psi_mod = make_uniform(psi_mod);

    auto state = make_uniform(0.0);

    std::cout << "φ+ψ CANCELLATION (1000 steps):\n";
    std::cout << "==============================\n\n";

    int errors = 0;
    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 1000; step++) {
        // Alternating φ at ψ — natural cancellation
        if (step % 2 == 0) {
            state = cc->EvalAdd(state, ct_phi_mod);
        } else {
            state = cc->EvalAdd(state, ct_psi_mod);
        }

        double v = decrypt_slot(state, 0);
        bool bounded = (v >= -0.1 && v <= 1.1);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "q=" << v
                      << " level=" << state->GetLevel()
                      << " elapsed=" << elapsed << "s"
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ φ+ψ BOUNDED!" : "❌") << "\n";
    std::cout << "========================================\n\n";

    std::cout << "EMERGENT PROPERTY:\n";
    std::cout << "==================\n\n";
    std::cout << "  φ_mod + ψ_mod = 0\n";
    std::cout << "  Ito ay natural na zero-sum\n";
    std::cout << "  Kaya ang alternating H at P ay bounded!\n";

    return 0;
}
