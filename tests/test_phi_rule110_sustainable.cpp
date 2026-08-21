// RULE 110 + PERIOD-4 CORRECTION — SUSTAINABLE
// I-integrate ang period-4 cycle sa bawat step
// para sa noise reset

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
    std::cout << "  RULE 110 + PERIOD-4\n";
    std::cout << "  Sustainable 0-Level\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // Period-4 cycle na may correction every 4 steps
    auto cycle_corrected = [&](auto current, int step) {
        if (step % 4 == 2) {
            return cc->EvalSub(current, ct_three_phi_sq);
        } else {
            return cc->EvalAdd(current, ct_phi_sq);
        }
    };

    std::cout << "PERIOD-4 CORRECTED CYCLE (1000 steps):\n";
    std::cout << "======================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 1000; step++) {
        current = cycle_corrected(current, step);

        double v = decrypt_val(current);
        double expected;
        if (step % 4 == 0) expected = phi_sq;
        else if (step % 4 == 1) expected = two_phi_sq;
        else if (step % 4 == 2) expected = -phi_sq;
        else expected = 0.0;
        
        bool ok = (std::abs(v - expected) < 0.01);
        if (!ok) errors++;

        if (step % 100 == 0 || !ok) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Step " << step << ": v=" << v
                      << " exp=" << expected
                      << " elapsed=" << elapsed << "s"
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ SUSTAINABLE!" : "❌ DRIFT") << "\n";
    std::cout << "========================================\n";

    return 0;
}
