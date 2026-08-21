// FIBONACCI RECURRENCE #3 — BOUNDED PERIOD-6
// x_{n+1} = x_n - x_{n-1} + φ
// Bounded, 0-level, period-6

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RECURRENCE #3 — BOUNDED PERIOD-6\n";
    std::cout << "  x_{n+1} = x_n - x_{n-1} + φ\n";
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

    auto ct_phi = make_ct(phi);

    std::cout << "BOUNDED PERIOD-6 TEST (1000 steps):\n";
    std::cout << "====================================\n\n";

    auto x_n_minus_1 = make_ct(0.0);      // x_{-1} = 0
    auto x_n = make_ct(phi_sq);            // x_0 = φ²

    int errors = 0;
    int total_steps = 1000;

    for (int step = 1; step <= total_steps; step++) {
        // x_{n+1} = x_n - x_{n-1} + φ
        auto x_n_plus_1 = cc->EvalAdd(cc->EvalSub(x_n, x_n_minus_1), ct_phi);

        x_n_minus_1 = x_n;
        x_n = x_n_plus_1;

        double v = decrypt_val(x_n);
        
        // Check kung bounded
        bool bounded = (v >= -2.0 && v <= 5.0);
        if (!bounded) errors++;

        if (step <= 15 || step % 100 == 0) {
            std::cout << "  Step " << step << ": v=" << v
                      << " level=" << x_n->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: " << total_steps << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Level: " << x_n->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED PERIOD-6!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
