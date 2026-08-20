// CKKS DEEP ALTERNATING REFRESH — 50+ Gates
// Depth 60, 5 gates per cycle, alternating refresh
// TARGET: 50+ gates na True FHE

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
    std::cout << "  CKKS DEEP ALTERNATING REFRESH\n";
    std::cout << "  50+ Gates True FHE\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    const double phi = 1.6180339887498948482;
    const double psi_10 = std::pow(1.0/phi, 10.0);
    const double phi_10 = std::pow(phi, 10.0);

    // Encrypted constants
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    std::vector<std::complex<double>> vec_psi(slots, {0.0, 0.0});
    vec_psi[0] = {psi_10, 0.0};
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi));

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_10, 0.0};
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));

    // NAND variants
    auto nand_1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };
    auto nand_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_phi);
        return cc->EvalSub(ct_psi, scaled);
    };

    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi); };
    auto refresh_phi = [&](auto ct) { return cc->EvalMult(ct, ct_phi); };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "DEEP TEST: 55 gates\n";
    std::cout << "===================\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int total_errors = 0;
    int total_gates = 55;
    int gates_per_cycle = 5;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        if (gate > 0 && gate % gates_per_cycle == 0) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
            } else {
                current = refresh_phi(current);
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
        if (!ok) total_errors++;

        if (gate < 3 || gate >= total_gates - 3 || gate % 10 == 9) {
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
    std::cout << "  Errors: " << total_errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - total_errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << " seconds\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
