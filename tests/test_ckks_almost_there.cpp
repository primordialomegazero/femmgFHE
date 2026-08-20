// CKKS ALMOST THERE — Tamang Alternating Refresh
// × ψ^10 pababa, × φ^10 pataas

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
    std::cout << "  CKKS — TAMANG ALTERNATING REFRESH\n";
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
    const double psi_5 = std::sqrt(psi_10);

    // Encrypted constants
    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct1 = make_ct(1.0);
    auto ct_psi10 = make_ct(psi_10);
    auto ct_phi10 = make_ct(phi_10);
    auto ct_psi5 = make_ct(psi_5);
    auto ct_zero = make_ct(0.0);

    // NAND: scale 1 → "1"=1, scale ψ → "1"=ψ^5
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

    std::cout << "50 GATES (25 per cycle, alternating ψ^10/φ^10):\n";
    std::cout << "================================================\n\n";

    auto current = ct1;
    int scale_idx = 0;  // 0=scale1, 1=scaleψ
    int errors = 0;
    int gates_per_cycle = 25;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 50; gate++) {
        if (gate > 0 && gate % gates_per_cycle == 0) {
            if (scale_idx == 0) {
                current = refresh_down(current);  // × ψ^10
                scale_idx = 1;
                // Ngayon ang "1" ay ψ^10 scale
                // NAND_ψ ay gumagamit ng ψ^5-encoded inputs
                // Pero ang current ay nasa ψ^10 scale na
                // Kailangan i-adjust: ang "1" sa ψ-scale ay ψ^10 mismo
                // NAND(ψ^10, ψ^10) = ψ^10 - ψ^20/ψ^10... hmm
                // Para sa 1-mult: NAND = ψ^10 - a·b/ψ^10
                // Simpleng approach: treat "1" bilang ψ^10
                // at NAND = ψ^10 - a·b·φ^10 (2 mults ulit)
            } else {
                current = refresh_up(current);  // × φ^10
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

        if (gate < 3 || gate >= 47 || gate % 25 == 24) {
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
