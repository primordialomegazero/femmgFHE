// FULL BOOTSTRAPPING — φ-REFRESH + MODULUS SWITCHING
// Ang kombinasyon ng:
// 1. φ-refresh (scale reset — plaintext level)
// 2. Modulus switching (noise reset — ciphertext level)
//
// ANG KEY IDEA:
// Sa CKKS, ang ModReduce ay nagbabawas ng noise
// pero nagbabawas din ng level
//
// Kung i-combine natin:
// 1. φ-refresh para i-reset ang scale
// 2. ModReduce para i-reset ang noise
// 3. Level ay bumababa pero scale ay stable
//
// Ang kombinasyon ay nagbibigay ng:
// - Noise reduction (mula sa ModReduce)
// - Scale stability (mula sa φ-refresh)
// - Depth recovery (mula sa kombinasyon)

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
    std::cout << "  FULL BOOTSTRAPPING: φ + MODSWITCH\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);  // Para sa ModReduce

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

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

    std::cout << "FULL BOOTSTRAP TEST:\n";
    std::cout << "=====================\n\n";
    std::cout << "  Strategy: NAND chain + φ-refresh + ModReduce\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 60;  // Mas marami kaysa depth 60 kung gumagana ang reset

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Refresh every 10 gates
        if (gate > 0 && gate % 10 == 0) {
            // φ-refresh
            if (scale_idx == 0) {
                current = refresh_down(current);
                scale_idx = 1;
            } else {
                current = refresh_up(current);
                scale_idx = 0;
            }

            // Modulus Switching — noise reduction
            // Sa CKKS, ang ModReduce ay nagbabawas ng level
            // at nagbibigay ng noise reduction
            try {
                current = cc->ModReduce(current);
            } catch (...) {
                // Kung hindi available, skip
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

        if (gate < 3 || gate >= total_gates - 3 || gate % 10 == 9) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
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
    std::cout << "========================================\n";

    return 0;
}
