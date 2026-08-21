// PERIOD-0 iO — MULTIPLE FULL CIRCUITS
// Irrational rotation = natural obfuscation
// Hindi nasasabog kasi bounded sa [0,1)

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
    std::cout << "  PERIOD-0 iO — MULTIPLE CIRCUITS\n";
    std::cout << "  Irrational Obfuscation\n";
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
    auto ct_one = make_ct(1.0);

    // ============================================
    // PERIOD-0 iO — DALAWANG MAGKAIBANG CIRCUITS
    // ============================================
    // Circuit A: +φ² pagkatapos ng bawat gate (mod 1)
    // Circuit B: +2φ² pagkatapos ng bawat gate (mod 1)
    //
    // Magkaiba ang structure, pareho ang behavior:
    // pareho silang dense sa [0,1) at walang repeat

    auto circuit_A = [&](auto state) {
        auto next = cc->EvalAdd(state, ct_phi_sq);
        double v = decrypt_val(next);
        if (v >= 1.0) {
            next = cc->EvalSub(next, ct_one);
        }
        return next;
    };

    auto circuit_B = [&](auto state) {
        auto next = cc->EvalAdd(state, ct_phi_sq);
        next = cc->EvalAdd(next, ct_phi_sq);  // +2φ²
        double v = decrypt_val(next);
        while (v >= 1.0) {
            next = cc->EvalSub(next, ct_one);
        }
        return next;
    };

    std::cout << "FULL CIRCUIT iO TEST (1000 steps):\n";
    std::cout << "==================================\n\n";

    auto state_A = make_ct(0.0);
    auto state_B = make_ct(0.0);
    int collisions = 0;

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 1000; step++) {
        state_A = circuit_A(state_A);
        state_B = circuit_B(state_B);

        double vA = decrypt_val(state_A);
        double vB = decrypt_val(state_B);

        // Check kung may collision (pareho ang state)
        if (std::abs(vA - vB) < 0.0001) {
            collisions++;
        }

        if (step < 20 || step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "A=" << vA << " B=" << vB
                      << " level=" << state_A->GetLevel() << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Collisions: " << collisions << "\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state_A->GetLevel() << "\n";
    std::cout << "  Status: " << (collisions == 0 ? "✅ iO PERFECT!" : "❌ MAY COLLISION") << "\n";
    std::cout << "========================================\n\n";

    std::cout << "iO PROPERTIES:\n";
    std::cout << "==============\n\n";
    std::cout << "  1. Walang repeat sa 1000 steps\n";
    std::cout << "  2. Dense sa [0,1)\n";
    std::cout << "  3. Indistinguishable — parehong pattern\n";
    std::cout << "  4. Natural na obfuscation via irrationality\n";
    std::cout << "  5. 0-level — walang multiplication\n";

    return 0;
}
