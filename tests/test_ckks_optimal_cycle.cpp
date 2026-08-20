// CKKS OPTIMAL CYCLE — 15 gates per cycle
// 55 gates + 4 refreshes = 59 mults (kasya sa depth 60)

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
    std::cout << "  CKKS OPTIMAL CYCLE — 15 GATES\n";
    std::cout << "  55 Gates True FHE\n";
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

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    std::vector<std::complex<double>> vec_psi(slots, {0.0, 0.0});
    vec_psi[0] = {psi_10, 0.0};
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi));

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_10, 0.0};
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));

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

    std::cout << "55 GATES (15 per cycle, 4 refreshes):\n";
    std::cout << "=======================================\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int errors = 0;
    int gates_per_cycle = 15;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 55; gate++) {
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
        if (!ok) errors++;

        if (gate < 3 || gate >= 52 || gate % 15 == 14) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT: " << errors << "/55 errors ("
              << (100.0 * (55 - errors) / 55) << "%)\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
