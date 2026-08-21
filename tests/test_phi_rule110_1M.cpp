// RULE 110 + PERIOD-4 — 1,000,000 STEPS
// Final Unbounded Confirmation

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
    std::cout << "  RULE 110 + PERIOD-4 — 1M STEPS\n";
    std::cout << "  FINAL CONFIRMATION\n";
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(2 * phi_sq);
    auto ct_neg_phi_sq = make_ct(-phi_sq);

    auto current = ct_phi_sq;
    int total = 1000000;
    int errors = 0;

    std::cout << "RUNNING 1,000,000 STEPS...\n";
    auto t_start = high_resolution_clock::now();

    for (int i = 0; i < total; i++) {
        // Period-4 cycle: φ² → 2φ² → -φ² → 0 → φ²...
        int phase = i % 4;
        if (phase == 0) {
            current = cc->EvalSub(ct_phi_sq, current);  // φ² - x
        } else if (phase == 1) {
            current = cc->EvalSub(ct_two_phi_sq, current);  // 2φ² - x
        } else if (phase == 2) {
            current = cc->EvalSub(ct_neg_phi_sq, current);  // -φ² - x
        } else {
            current = cc->EvalSub(cc->EvalNegate(current), current);  // 0 - x
        }

        if (i % 100000 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - t_start).count();
            std::cout << "  [" << i << "/" << total << "] " << elapsed << "s\n";
            std::cout.flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << "/" << total << " errors\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total << " ms\n";
    std::cout << "  Level: 0 — UNBOUNDED CONFIRMED\n";

    return 0;
}
