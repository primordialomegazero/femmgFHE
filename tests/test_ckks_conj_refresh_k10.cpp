// CKKS + φ-CONJUGATE REFRESH — k=10
// Ang refresh ay 1 multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ-CONJUGATE REFRESH (k=10)\n";
    std::cout << "========================================\n\n";

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

    // φ^10 ≈ 122.992, ψ^10 ≈ 0.00813
    const double phi_10 = std::pow(1.6180339887498948482, 10.0);
    const double psi_10 = std::pow(1.0/1.6180339887498948482, 10.0);

    std::cout << "φ^10 = " << phi_10 << "\n";
    std::cout << "ψ^10 = " << psi_10 << "\n\n";

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_10, 0.0};

    std::vector<std::complex<double>> vec_psi(slots, {0.0, 0.0});
    vec_psi[0] = {psi_10, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    // φ-NAND: φ - a·b·φ⁻¹ = φ - a·b·ψ
    auto phi_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_psi);
        return cc->EvalSub(ct_phi, scaled);
    };

    // Conjugate refresh: ct → ct·ψ^10
    auto conj_refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "TEST 1: CONJUGATE REFRESH\n";
    std::cout << "==========================\n\n";

    auto refreshed = conj_refresh(ct_phi);
    double val = decrypt_val(refreshed);
    std::cout << "  refresh(φ^10) = " << val << " (expected 1)\n";
    std::cout << "  Match: " << (std::abs(val - 1.0) < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    std::cout << "TEST 2: NAND CHAIN (10 gates)\n";
    std::cout << "==============================\n\n";

    auto current = ct_phi;
    int errors = 0;

    for (int gate = 0; gate < 10; gate++) {
        current = phi_nand(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_10;
        bool ok = std::abs(v - expected) < 0.15 * phi_10;

        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": val=" << v
                  << " expected=" << expected
                  << (ok ? " YES" : " NO") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/10\n";

    return 0;
}
