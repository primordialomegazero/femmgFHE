// φ-HOMOMORPHIC EVALUATION — Replace Their Bootstrapping
// Ang CKKS ciphertext ay polynomial ct(x) = c₀ + c₁x + c₂x² + ...
// Kailangan nating i-evaluate ang ct sa φ at ψ homomorphically
//
// ANG KEY:
// Sa CKKS, ang ciphertext ay nasa polynomial ring
// Ang evaluation sa φ at ψ ay:
//   ct(φ) = c₀ + c₁φ + c₂φ² + ...
//   ct(ψ) = c₀ + c₁ψ + c₂ψ² + ...
//
// Sa encrypted domain, ito ay:
//   E(ct(φ)) = E(c₀) + E(c₁)·φ + E(c₂)·φ² + ...
//
// ANG STRATEGY:
// 1. Magkaroon ng encrypted ciphertext coefficients
// 2. I-multiply ang bawat coefficient sa φ^j o ψ^j
// 3. I-sum ang lahat para makuha ang evaluated value

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
    std::cout << "  φ-HOMOMORPHIC EVALUATION\n";
    std::cout << "  Replace Their Bootstrapping\n";
    std::cout << "========================================\n\n";

    // CKKS Setup — depth 30 para sa experiment
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

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

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

    std::cout << "φ = " << phi << ", ψ = " << psi << "\n\n";

    // ============================================
    // SIMPLE TEST: ct(x) = m + e·x
    // ct(φ) = m + e·φ
    // ct(ψ) = m + e·ψ
    // ============================================
    std::cout << "SIMPLE TEST: ct(x) = m + e·x\n";
    std::cout << "==============================\n\n";

    // Encrypted m at e
    auto ct_m = make_ct(1.0);   // m=1
    auto ct_e = make_ct(5.0);   // e=5

    // Encrypted φ at ψ (plaintext constants)
    auto ct_phi_const = make_ct(phi);
    auto ct_psi_const = make_ct(psi);

    // ct(φ) = m + e·φ
    auto e_times_phi = cc->EvalMult(ct_e, ct_phi_const);
    auto ct_at_phi = cc->EvalAdd(ct_m, e_times_phi);

    // ct(ψ) = m + e·ψ
    auto e_times_psi = cc->EvalMult(ct_e, ct_psi_const);
    auto ct_at_psi = cc->EvalAdd(ct_m, e_times_psi);

    double val_phi = decrypt_val(ct_at_phi);
    double val_psi = decrypt_val(ct_at_psi);

    std::cout << "  ct(φ) = " << val_phi << " (expected " << (1.0 + 5.0 * phi) << ")\n";
    std::cout << "  ct(ψ) = " << val_psi << " (expected " << (1.0 + 5.0 * psi) << ")\n\n";

    // ============================================
    // NOISE SEPARATION SA ENCRYPTED DOMAIN
    // ============================================
    std::cout << "ENCRYPTED NOISE SEPARATION:\n";
    std::cout << "===========================\n\n";

    // diff = ct(φ) - ct(ψ)
    auto diff = cc->EvalSub(ct_at_phi, ct_at_psi);
    double val_diff = decrypt_val(diff);

    // sum = ct(φ) + ct(ψ)
    auto sum = cc->EvalAdd(ct_at_phi, ct_at_psi);
    double val_sum = decrypt_val(sum);

    std::cout << "  diff = " << val_diff << " (expected " << (5.0 * (phi - psi)) << " = e·√5)\n";
    std::cout << "  sum = " << val_sum << " (expected " << (2.0 + 5.0 * (phi + psi)) << " = 2m + e)\n\n";

    // ============================================
    // RECOVERY SA ENCRYPTED DOMAIN
    // ============================================
    std::cout << "ENCRYPTED RECOVERY:\n";
    std::cout << "====================\n\n";

    // e = diff / √5
    const double sqrt5 = std::sqrt(5.0);
    auto ct_sqrt5 = make_ct(sqrt5);
    auto ct_inv_sqrt5 = make_ct(1.0 / sqrt5);

    auto e_recovered = cc->EvalMult(diff, ct_inv_sqrt5);
    double val_e = decrypt_val(e_recovered);
    std::cout << "  Recovered e = " << val_e << " (expected 5)\n";

    // m = (sum - e) / 2
    auto sum_minus_e = cc->EvalSub(sum, e_recovered);
    auto ct_inv2 = make_ct(0.5);
    auto m_recovered = cc->EvalMult(sum_minus_e, ct_inv2);
    double val_m = decrypt_val(m_recovered);
    std::cout << "  Recovered m = " << val_m << " (expected 1)\n\n";

    // ============================================
    // ANG KEY: ITO BA AY BOOTSTRAPPING?
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  ANG KEY QUESTION:\n";
    std::cout << "  Ito ba ay bootstrapping?\n\n";
    std::cout << "  OO KUNG:\n";
    std::cout << "  1. Ang diff at sum ay nagbibigay ng\n";
    std::cout << "     exact na noise at signal\n";
    std::cout << "  2. Ang recovery ay homomorphic\n";
    std::cout << "  3. Ang noise ay na-reset\n\n";
    std::cout << "  HINDI PA KUNG:\n";
    std::cout << "  1. Ang ciphertext noise ay hindi pa\n";
    std::cout << "     na-reset sa polynomial level\n";
    std::cout << "  2. Ito ay value-level recovery lang\n";
    std::cout << "========================================\n";

    return 0;
}
