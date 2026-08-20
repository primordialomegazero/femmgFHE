// CKKS LAZY REFRESH — φ-THRESHOLD
// Mag-refresh lang kapag ang noise ay umabot sa 61.8%
// Hindi fixed interval — adaptive sa actual noise level
//
// ANG KEY:
// Sa halip na "every 25 gates", gamitin ang:
// "refresh kapag noise > 61.8% ng budget"
//
// Ito ay mas efficient kasi:
// - Simula: mabagal ang noise growth → mas kaunting refresh
// - Dulo: mabilis ang noise growth → mas maraming refresh
// - Total: optimized refresh schedule

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
    std::cout << "  CKKS LAZY REFRESH — φ-THRESHOLD\n";
    std::cout << "  Adaptive Refresh Schedule\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

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

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto ct_one = make_ct(1.0);
        return cc->EvalSub(ct_one, prod);
    };

    // φ/ψ refresh
    const double psi_10 = std::pow(1.0/1.6180339887498948482, 10.0);
    const double phi_10 = std::pow(1.6180339887498948482, 10.0);
    auto ct_psi10 = make_ct(psi_10);
    auto ct_phi10 = make_ct(phi_10);

    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi10); };
    auto refresh_phi = [&](auto ct) { return cc->EvalMult(ct, ct_phi10); };

    std::cout << "LAZY REFRESH TEST:\n";
    std::cout << "===================\n\n";
    std::cout << "  Threshold: 61.8% ng noise budget\n";
    std::cout << "  Refresh lang kapag lumampas\n\n";

    auto current = make_ct(1.0);
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 50;
    int refresh_count = 0;
    double noise_threshold = 0.618;  // 1/φ

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // I-estimate ang noise mula sa level
        int level = current->GetLevel();
        double noise_estimate = 1.0 - (double)level / 60.0;  // Normalized

        // Lazy refresh: kapag ang noise estimate ay lumampas sa threshold
        if (noise_estimate > noise_threshold && gate < total_gates - 1) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
            } else {
                current = refresh_phi(current);
                scale_idx = 0;
            }
            refresh_count++;
        }

        // Verify every 10 gates
        if (gate % 10 == 0 || gate >= total_gates - 2) {
            double v = decrypt_val(current);
            double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
            int expected_int = (gate % 2 == 0) ? 0 : 1;
            double expected = expected_int ? scale_factor : 0.0;
            int got = (std::abs(v) > scale_factor * 0.5) ? 1 : 0;

            if (got != expected_int) errors++;

            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << level
                      << " noise=" << noise_estimate
                      << (got == expected_int ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Refreshes: " << refresh_count << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
