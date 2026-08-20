// CKKS ALTERNATING φ/ψ REFRESH
// Scale ay naka-bounce sa pagitan ng 1 at ψ^10
// 
// CYCLE:
// 1. NAND_1 (scale 1)
// 2. Refresh ψ: ct → ct·ψ^10 (scale → ψ^10)
// 3. NAND_ψ (scale ψ^10)
// 4. Refresh φ: ct → ct·φ^10 (scale → 1)
// 5. Ulitin
//
// φ^10 · ψ^10 = 1 kaya bumabalik sa original scale!

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
    std::cout << "  CKKS ALTERNATING φ/ψ REFRESH\n";
    std::cout << "  Scale Bounded sa [ψ^10, 1]\n";
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

    std::cout << "ψ^10 = " << psi_10 << "\n";
    std::cout << "φ^10 = " << phi_10 << "\n";
    std::cout << "φ^10 · ψ^10 = " << phi_10 * psi_10 << " (dapat 1)\n\n";

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

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    // NAND variants
    auto nand_scale1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto nand_scale_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_phi);  // × φ^10 = 1/ψ^10
        return cc->EvalSub(ct_psi, scaled);
    };

    // Refresh operations
    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi); };
    auto refresh_phi = [&](auto ct) { return cc->EvalMult(ct, ct_phi); };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "ALTERNATING REFRESH TEST:\n";
    std::cout << "==========================\n\n";

    auto current = ct1;
    int scale_idx = 0;  // 0=scale1, 1=scaleψ
    int total_errors = 0;
    int total_gates = 0;
    int gates_in_cycle = 5;  // 5 gates per cycle para sa testing

    std::cout << "Cycle 1 (Scale 1):\n";

    for (int gate = 0; gate < 25; gate++) {
        // Determine scale
        if (gate > 0 && gate % gates_in_cycle == 0) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
                std::cout << "\n  Refresh ψ → Scale ψ^10\n";
            } else {
                current = refresh_phi(current);
                scale_idx = 0;
                std::cout << "\n  Refresh φ → Scale 1\n";
            }
        }

        // NAND sa kasalukuyang scale
        if (scale_idx == 0) {
            current = nand_scale1(current, current);
        } else {
            current = nand_scale_psi(current, current);
        }

        // Verify
        double v = decrypt_val(current);
        double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
        int is_odd = (gate % 2 == 1);
        double expected = is_odd ? scale_factor : 0.0;

        bool ok = (std::abs(v - expected) < 0.05 * std::max(1.0, scale_factor));
        if (!ok) total_errors++;
        total_gates++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " expected=" << expected
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Total gates: " << total_gates << "\n";
    std::cout << "  Total errors: " << total_errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - total_errors) / total_gates) << "%\n";
    std::cout << "========================================\n";

    return 0;
}
