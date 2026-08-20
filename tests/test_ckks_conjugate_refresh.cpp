// CKKS + φ-CONJUGATE REFRESH
// Ang refresh ay 1 multiplication: ct → ct · ψ^k
//
// ANG TEORYA:
// Sa CKKS, ang noise ay nasa polynomial coefficients
// Ang ψ^k multiplication ay dapat magre-rescale ng noise
// Kung ang signal ay nasa φ^k-scale at ang noise ay nasa ψ^k-scale,
// ang refresh ay dapat magbalik ng signal sa 1-scale
//
// TEST:
// 1. NAND chain na may φ-scaling
// 2. Conjugate refresh tuwing 10 gates
// 3. I-compare sa walang refresh

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ-CONJUGATE REFRESH\n";
    std::cout << "  1 Multiplication Refresh\n";
    std::cout << "========================================\n\n";

    // CKKS Setup — depth 30 muna para mabilis
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::cout << "CKKS: slots=" << slots << "\n\n";

    // φ-Structure sa plaintext
    const double phi_val = 1.6180339887498948482;
    const double psi_val = 1.0 / phi_val;  // ψ = φ⁻¹ ≈ 0.618

    // ============================================
    // ENCODING: "1" → φ^k scale
    // ============================================
    std::vector<std::complex<double>> vec_phi_k(slots, {0.0, 0.0});
    vec_phi_k[0] = {std::pow(phi_val, 42.0), 0.0};  // φ^42

    std::vector<std::complex<double>> vec_psi_k(slots, {0.0, 0.0});
    vec_psi_k[0] = {std::pow(psi_val, 42.0), 0.0};  // ψ^42

    std::vector<std::complex<double>> vec_one(slots, {0.0, 0.0});
    vec_one[0] = {1.0, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi_k));
    auto ct_psi_k = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi_k));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    // NAND sa φ^k scale: φ^k - a·b·φ^(-k)
    auto phi_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto inv_phi_k = ct_psi_k;  // ψ^k = φ^(-k)
        auto scaled = cc->EvalMult(prod, inv_phi_k);
        return cc->EvalSub(ct_phi_k, scaled);
    };

    // Conjugate refresh: ct → ct · ψ^k
    auto conjugate_refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi_k);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "φ^42 ≈ " << std::pow(phi_val, 42.0) << "\n";
    std::cout << "ψ^42 ≈ " << std::pow(psi_val, 42.0) << "\n\n";

    // ============================================
    // TEST 1: CONJUGATE REFRESH
    // ============================================
    std::cout << "TEST 1: CONJUGATE REFRESH\n";
    std::cout << "==========================\n\n";

    // refresh(φ^k) = φ^k · ψ^k = 1
    auto refreshed = conjugate_refresh(ct_phi_k);
    double val_refreshed = decrypt_val(refreshed);
    std::cout << "  refresh(φ^k) = " << val_refreshed << " (expected 1)\n";

    // refresh(1) = ψ^k
    auto refreshed_one = conjugate_refresh(ct_one);
    double val_refreshed_one = decrypt_val(refreshed_one);
    std::cout << "  refresh(1) = " << val_refreshed_one << " (expected " << std::pow(psi_val, 42.0) << ")\n\n";

    // ============================================
    // TEST 2: NAND CHAIN NA MAY REFRESH
    // ============================================
    std::cout << "TEST 2: NAND CHAIN + REFRESH\n";
    std::cout << "=============================\n\n";

    // Strategy:
    // 1. 10 φ-NAND gates (20 multiplications)
    // 2. Conjugate refresh (1 multiplication)
    // 3. Ulitin

    auto current = ct_phi_k;  // Start sa "1" (φ^k scale)
    int errors = 0;
    int total_gates = 12;  // 12 gates = 24 mults + 2 refreshes = 26 mults

    std::cout << "  Gate | Value | Expected | OK?\n";
    std::cout << "  -----|-------|----------|-----\n";

    for (int gate = 0; gate < total_gates; gate++) {
        current = phi_nand(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : std::pow(phi_val, 42.0);
        bool ok = std::abs(v - expected) < 0.15 * std::pow(phi_val, 42.0);

        if (!ok) errors++;

        std::cout << "  " << gate << "    | " << v
                  << " | " << expected
                  << " | " << (ok ? "YES" : "NO") << "\n";

        // Refresh every 5 gates
        if ((gate + 1) % 5 == 0 && gate < total_gates - 1) {
            current = conjugate_refresh(current);
        }
    }

    std::cout << "\n  Errors: " << errors << "/" << total_gates << "\n";

    return 0;
}
