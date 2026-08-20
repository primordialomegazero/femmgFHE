// CKKS MULTI-CYCLE — φ-Refresh sa Bawat Scale
// Cycle 1: Scale 1 (NAND_1)
// Cycle 2: Scale ψ^10 (NAND_ψ)
// Cycle 3: Scale ψ^20 (NAND_ψ²)
// ...
//
// ANG TARGET: 50+ gates sa depth 60

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
    std::cout << "  CKKS MULTI-CYCLE φ-REFRESH\n";
    std::cout << "  50+ Gates Target\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
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

    std::cout << "ψ^10 = " << psi_10 << "\n\n";

    // Precompute encrypted constants para sa iba't ibang scales
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    // ψ^10, ψ^20, ψ^30, ψ^40 (para sa 5 cycles)
    std::vector<Ciphertext<DCRTPoly>> ct_psi_scales;
    for (int c = 1; c <= 5; c++) {
        double psi_val = std::pow(psi_10, c);
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {psi_val, 0.0};
        ct_psi_scales.push_back(cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec)));
    }

    // φ^10, φ^20, φ^30, φ^40 (para sa NAND scaling)
    std::vector<Ciphertext<DCRTPoly>> ct_phi_scales;
    for (int c = 1; c <= 5; c++) {
        double phi_val = std::pow(1.0/psi_10, c);
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {phi_val, 0.0};
        ct_phi_scales.push_back(cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec)));
    }

    // NAND sa iba't ibang scales
    auto nand_at_scale = [&](auto a, auto b, int scale_idx) {
        // scale_idx 0 = scale 1, 1 = scale ψ^10, 2 = scale ψ^20, ...
        if (scale_idx == 0) {
            auto prod = cc->EvalMult(a, b);
            return cc->EvalSub(ct1, prod);
        } else {
            auto prod = cc->EvalMult(a, b);
            auto scaled = cc->EvalMult(prod, ct_phi_scales[scale_idx - 1]);
            return cc->EvalSub(ct_psi_scales[scale_idx - 1], scaled);
        }
    };

    auto refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi_scales[0]);  // × ψ^10
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // MULTI-CYCLE TEST
    // ============================================
    std::cout << "MULTI-CYCLE TEST (10 gates per cycle):\n";
    std::cout << "========================================\n\n";

    auto current = ct1;
    int scale_idx = 0;
    int total_errors = 0;
    int total_gates = 0;
    int cycle = 1;

    std::cout << "Cycle " << cycle << " (Scale 1):\n";

    for (int gate = 0; gate < 50; gate++) {
        // Decide kung anong scale
        if (gate > 0 && gate % 10 == 0) {
            // Refresh at bagong cycle
            current = refresh(current);
            scale_idx++;
            cycle++;

            double scale_val = std::pow(psi_10, scale_idx);
            std::cout << "\n  Refresh → Scale " << scale_val << "\n";
            std::cout << "Cycle " << cycle << " (Scale " << scale_val << "):\n";
        }

        // NAND sa kasalukuyang scale
        current = nand_at_scale(current, current, scale_idx);

        // Decrypt at check
        double v = decrypt_val(current);

        // Ang expected value ay depende sa scale at gate parity
        double scale_factor = (scale_idx == 0) ? 1.0 : std::pow(psi_10, scale_idx);
        int is_odd = (gate % 2 == 1);  // 1 kung ang expected ay "true"
        
        // Sa even gates: expected = 0
        // Sa odd gates: expected = scale_factor (ang "true" value sa scale na ito)
        double expected = is_odd ? scale_factor : 0.0;

        bool ok = (std::abs(v - expected) < 0.05 * std::max(1.0, scale_factor));
        if (!ok) total_errors++;
        total_gates++;

        if (gate < 5 || gate >= 45) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " expected=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Total gates: " << total_gates << "\n";
    std::cout << "  Total errors: " << total_errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - total_errors) / total_gates) << "%\n";
    std::cout << "========================================\n";

    return 0;
}
