// φ + MODREDUCE + EVALBOOTSTRAP — FIXED PARAMETERS
// Depth 10, levelBudget {5,4} para sa mas malaking correction factor

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
    std::cout << "  FULL COMBO V2 — FIXED PARAMS\n";
    std::cout << "========================================\n\n";

    // Depth 10 para sa manageable bootstrap
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Bootstrapping setup na may mas malaking budget
    uint32_t slots = 512;
    std::vector<uint32_t> levelBudget = {5, 4};
    std::vector<uint32_t> dim1 = {0, 0};
    cc->EvalBootstrapSetup(levelBudget, dim1, slots);
    cc->EvalBootstrapKeyGen(keys.secretKey, slots);

    std::cout << "Bootstrap setup: COMPLETE\n\n";

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

    std::cout << "FULL BOOTSTRAP TEST (20 gates):\n";
    std::cout << "=================================\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 20;
    int bootstrap_count = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Every 5 gates: full bootstrap
        if (gate > 0 && gate % 5 == 0) {
            // φ-refresh
            if (scale_idx == 0) {
                current = refresh_down(current);
                scale_idx = 1;
            } else {
                current = refresh_up(current);
                scale_idx = 0;
            }

            // EvalBootstrap para sa level reset
            try {
                current = cc->EvalBootstrap(current);
                bootstrap_count++;
                std::cout << "  [Bootstrap " << bootstrap_count << " sa gate " << gate << "]\n";
            } catch (std::exception& e) {
                std::cout << "  Bootstrap FAILED: " << e.what() << "\n";
                break;
            }
        }

        // NAND
        if (scale_idx == 0) {
            current = nand_1(current, current);
        } else {
            current = nand_psi(current, current);
        }

        // Verify
        double v = decrypt_val(current);
        double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
        int is_odd = (gate % 2 == 1);
        double expected = is_odd ? scale_factor : 0.0;

        bool ok = (std::abs(v - expected) < 0.05 * std::max(1.0, scale_factor));
        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << " level=" << current->GetLevel()
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Bootstraps: " << bootstrap_count << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
