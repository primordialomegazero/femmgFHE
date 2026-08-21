// φ-NORMALIZED CYCLIC NAND
// Ang problema kanina: scale drift dahil sa φ/ψ bridge
// Ngayon: normalize ang scale pagkatapos ng bawat bridge
//
// RULE:
// Pagkatapos ng NAND, ang value ay 0 o 1
// Bridge φ: value × φ → pwedeng maging 1.618 o 0
// Bridge ψ: value × ψ → pwedeng maging 0.618 o 0
//
// Kaya kailangan ng NORMALIZATION:
// φ-bridge output: kung value > 1, ibalik sa 1
// ψ-bridge output: kung 0 < value < 1, ibalik sa 0 (o panatilihin)
//
// PERO homomorphic dapat — hindi decrypt-check-encrypt
// Kaya gagamit tayo ng φ-normalized NAND formula mismo

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
    std::cout << "  φ-NORMALIZED CYCLIC NAND\n";
    std::cout << "  Homomorphic Scale Correction\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

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

    auto ct_one = make_ct(1.0);
    auto ct_zero = make_ct(0.0);
    auto ct_phi = make_ct(phi);
    auto ct_psi = make_ct(psi);

    // ============================================
    // φ-NORMALIZED NAND
    // ============================================
    // Sa halip na standard NAND: 1 - a·b
    // Gagamit tayo ng φ-normalized version:
    // NAND_φ(a,b) = φ - a·b·ψ
    // Kung a,b ∈ {0,1}: 
    //   NAND(0,0) = φ
    //   NAND(0,1) = φ
    //   NAND(1,1) = φ - ψ = 1
    //
    // Tapos i-normalize natin pabalik sa {0,1}:
    // normalize_φ(x) = x · ψ
    //   φ · ψ = -1
    //   1 · ψ = 0.618
    //
    // PERO ang period-2 ay nasa φ-domain:
    //   NOT_φ(x) = NAND_φ(x,x)
    //   NOT_φ(φ) = NAND_φ(φ,φ) = φ - φ²·ψ = φ - φ = 0
    //   NOT_φ(0) = NAND_φ(0,0) = φ
    //   NOT_φ(NOT_φ(x)) = x ✓ period-2!

    // Kaya ang "1" ay φ at ang "0" ay 0 sa φ-domain
    // At ang NAND_φ ay may period-2

    auto nand_phi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);      // a·b
        auto scaled = cc->EvalMult(prod, ct_psi); // a·b·ψ
        return cc->EvalSub(ct_phi, scaled);   // φ - a·b·ψ
    };

    // ============================================
    // φ-DOMAIN TEST
    // ============================================
    std::cout << "φ-DOMAIN NAND TRUTH TABLE:\n";
    std::cout << "===========================\n\n";

    auto t00 = nand_phi(ct_zero, ct_zero);
    auto t01 = nand_phi(ct_zero, ct_one);
    auto t11 = nand_phi(ct_one, ct_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(t01) << " (expected " << phi << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(t11) << " (expected 0)\n\n";

    // PERIOD-2 TEST
    std::cout << "PERIOD-2 SA φ-DOMAIN:\n";
    std::cout << "======================\n\n";

    auto current = make_ct(phi);  // "1" = φ
    int errors = 0;
    int total_gates = 20;  // 2 mults per gate, 40 mults sa depth 30

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_phi(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        if (gate < 5 || gate >= total_gates - 3) {
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
