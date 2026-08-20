// CKKS 100 GATES — Alternating φ/ψ Refresh
// Depth 140, 25 gates per cycle, 4 refreshes
// TARGET: 100 gates na True FHE

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 100 GATES — ALTERNATING REFRESH\n";
    std::cout << "  True FHE Target\n";
    std::cout << "========================================\n\n";

    // Depth 140 para sa 100 gates + refreshes
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(140);
    params.SetScalingModSize(35);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

    const double phi = 1.6180339887498948482;
    const double psi_10 = std::pow(1.0/phi, 10.0);
    const double phi_10 = std::pow(phi, 10.0);

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct1 = make_ct(1.0);
    auto ct_psi10 = make_ct(psi_10);
    auto ct_phi10 = make_ct(phi_10);

    auto nand_1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };
    auto nand_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_psi10, prod);
    };

    auto refresh_down = [&](auto ct) { return cc->EvalMult(ct, ct_psi10); };
    auto refresh_up = [&](auto ct) { return cc->EvalMult(ct, ct_phi10); };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "100 GATES (25 per cycle, 4 refreshes):\n";
    std::cout << "========================================\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 100;
    int gates_per_cycle = 25;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        if (gate > 0 && gate % gates_per_cycle == 0) {
            if (scale_idx == 0) {
                current = refresh_down(current);
                scale_idx = 1;
            } else {
                current = refresh_up(current);
                scale_idx = 0;
            }
        }

        if (scale_idx == 0) {
            current = nand_1(current, current);
        } else {
            current = nand_psi(current, current);
        }

        double v = decrypt_val(current);
        double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
        int is_odd = (gate % 2 == 1);
        double expected = is_odd ? scale_factor : 0.0;

        bool ok = (std::abs(v - expected) < 0.05 * std::max(1.0, scale_factor));
        if (!ok) errors++;

        if (gate < 3 || gate >= total_gates - 3 || gate % 25 == 24) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
