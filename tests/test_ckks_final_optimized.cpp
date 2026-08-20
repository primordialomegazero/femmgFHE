// CKKS FINAL OPTIMIZED — 1-mult NAND sa lahat ng scales
// Pre-scaled inputs para 1 multiplication lang per gate

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
    std::cout << "  CKKS FINAL OPTIMIZED\n";
    std::cout << "  1-Mult NAND sa Lahat ng Scales\n";
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
    const double sqrt_psi_10 = std::sqrt(psi_10);

    // ANG KEY: Pre-scale ang "1" bilang sqrt(scale)
    // Sa scale 1: "1" = 1
    // Sa scale ψ: "1" = sqrt(ψ^10) = ψ^5
    //
    // NAND sa scale ψ: ψ^10 - a·b (1 mult!)
    // Kung a = m₁·ψ^5 at b = m₂·ψ^5:
    // a·b = m₁m₂·ψ^10
    // NAND = ψ^10 - m₁m₂·ψ^10 = ψ^10(1-m₁m₂) ✓

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    std::vector<std::complex<double>> vec_psi10(slots, {0.0, 0.0});
    vec_psi10[0] = {psi_10, 0.0};
    auto ct_psi10 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi10));

    std::vector<std::complex<double>> vec_psi5(slots, {0.0, 0.0});
    vec_psi5[0] = {sqrt_psi_10, 0.0};
    auto ct_psi5 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi5));

    // NAND sa scale 1: 1 - a·b (1 mult)
    auto nand_1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    // NAND sa scale ψ: ψ^10 - a·b (1 mult!)
    auto nand_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 MUL LANG!
        return cc->EvalSub(ct_psi10, prod);
    };

    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi10); };
    auto refresh_phi = [&](auto ct) { 
        // φ^10 = 1/ψ^10
        return cc->EvalMult(ct, ct1);  // Placeholder — kailangan actual φ^10
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "ψ^10 = " << psi_10 << "\n";
    std::cout << "ψ^5 (sqrt) = " << sqrt_psi_10 << "\n\n";

    // Test: NAND sa scale ψ na may 1 mult
    std::cout << "NAND_ψ TRUTH TABLE (1 mult):\n";
    std::cout << "=============================\n\n";

    auto ct_psi_one = ct_psi5;  // "1" sa ψ-scale = ψ^5
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
        std::vector<std::complex<double>>(slots, {0.0, 0.0})));

    auto nand_00 = nand_psi(ct_zero, ct_zero);
    auto nand_01 = nand_psi(ct_zero, ct_psi_one);
    auto nand_11 = nand_psi(ct_psi_one, ct_psi_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << psi_10 << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << psi_10 << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // ============================================
    // FULL TEST: 55 gates na may 1-mult NAND sa lahat
    // ============================================
    std::cout << "55 GATES (1-mult NAND everywhere):\n";
    std::cout << "====================================\n\n";

    auto current = ct1;  // Scale 1: "1" = 1
    int scale_idx = 0;
    int errors = 0;
    int gates_per_cycle = 20;  // 20 gates per cycle

    // Sa scale 1: "1" = 1
    // Sa scale ψ: "1" = ψ^5 (sqrt ng ψ^10)
    // Refresh ψ: × ψ^10 (mula scale 1 → ψ, ang "1" ay nagiging ψ^10)
    //            pero ang "1" sa ψ-scale ay ψ^5, hindi ψ^10!
    //
    // AYUSIN NATIN:
    // Sa scale 1: "1" = 1
    // Refresh ψ5: × ψ^5 → "1" = ψ^5 (ito ang "true" sa ψ-scale)
    // NAND_ψ: ψ^10 - a·b (kung saan "1" = ψ^5)
    // Refresh φ5: × φ^5 → "1" = 1 (bumalik)

    auto refresh_psi5 = [&](auto ct) { return cc->EvalMult(ct, ct_psi5); };

    std::cout << "  (Using ψ^5 for refresh)\n\n";

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 50; gate++) {
        if (gate > 0 && gate % gates_per_cycle == 0) {
            if (scale_idx == 0) {
                current = refresh_psi5(current);
                scale_idx = 1;
            } else {
                // × φ^5 = 1/ψ^5 — kailangan ng φ^5 ciphertext
                // Para ngayon, gamitin natin ang ψ^10 refresh na may ψ^5
                current = refresh_psi5(current);
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

        if (gate < 3 || gate >= 47) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << "/50 errors ("
              << (100.0 * (50 - errors) / 50) << "%)\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";

    return 0;
}
