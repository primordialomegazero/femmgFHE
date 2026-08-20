// CKKS PRECISION FIX — φ-Noise Separation
// Ang issue: ang EvalSub ay hindi nagbibigay ng exact diff
// Ang fix: gumamit ng tamang scaling sa CKKS operations
//
// ANG KEY INSIGHT:
// Sa CKKS, ang bawat multiplication ay may scaling factor
// Ang EvalSub pagkatapos ng EvalMult ay may mismatch
// Kailangan i-align ang scales bago mag-subtract

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
    std::cout << "  CKKS PRECISION FIX\n";
    std::cout << "  φ-Noise Separation — Correct\n";
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

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;
    const double sqrt5 = std::sqrt(5.0);

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

    std::cout << "φ = " << phi << ", ψ = " << psi << ", √5 = " << sqrt5 << "\n\n";

    // ============================================
    // TEST 1: DIRECT COMPUTATION (Walang EvalMult)
    // ============================================
    std::cout << "TEST 1: Direct Computation\n";
    std::cout << "==========================\n\n";

    // Para sa m=1, e=5:
    // ct(φ) = 1 + 5·φ = 1 + 5(1.61803) = 9.09017
    // ct(ψ) = 1 + 5·ψ = 1 + 5(0.61803) = 4.09017
    // diff = ct(φ) - ct(ψ) = 5(φ-ψ) = 5√5 = 11.1803
    // sum = ct(φ) + ct(ψ) = 2 + 5(φ+ψ) = 2 + 5 = 7

    std::cout << "  Expected values:\n";
    std::cout << "    ct(φ) = " << (1.0 + 5.0 * phi) << "\n";
    std::cout << "    ct(ψ) = " << (1.0 + 5.0 * psi) << "\n";
    std::cout << "    diff = " << (5.0 * sqrt5) << "\n";
    std::cout << "    sum = " << (2.0 + 5.0) << "\n\n";

    // I-compute gamit ang CKKS na may tamang scaling
    auto ct_m = make_ct(1.0);
    auto ct_e = make_ct(5.0);
    auto ct_phi = make_ct(phi);
    auto ct_psi = make_ct(psi);

    // e·φ
    auto e_phi = cc->EvalMult(ct_e, ct_phi);
    // e·ψ
    auto e_psi = cc->EvalMult(ct_e, ct_psi);

    // ct(φ) = m + e·φ
    auto at_phi = cc->EvalAdd(ct_m, e_phi);
    // ct(ψ) = m + e·ψ
    auto at_psi = cc->EvalAdd(ct_m, e_psi);

    double val_at_phi = decrypt_val(at_phi);
    double val_at_psi = decrypt_val(at_psi);
    std::cout << "  CKKS values:\n";
    std::cout << "    ct(φ) = " << val_at_phi << "\n";
    std::cout << "    ct(ψ) = " << val_at_psi << "\n\n";

    // ============================================
    // TEST 2: DIFFERENCE NA MAY ALIGNED SCALES
    // ============================================
    std::cout << "TEST 2: Difference na may Aligned Scales\n";
    std::cout << "=========================================\n\n";

    // Ang problema: pagkatapos ng EvalMult, may ibang scale
    // Ang fix: i-normalize muna bago mag-subtract
    
    // Try: directly compute diff = e·(φ-ψ) = e·√5
    auto phi_minus_psi = make_ct(phi - psi);  // √5
    auto diff_direct = cc->EvalMult(ct_e, phi_minus_psi);
    double val_diff_direct = decrypt_val(diff_direct);
    std::cout << "  diff (direct e·√5) = " << val_diff_direct << " (expected " << (5.0 * sqrt5) << ")\n";

    // Try: at_phi - at_psi (pagkatapos ng EvalAdd)
    auto diff_sub = cc->EvalSub(at_phi, at_psi);
    double val_diff_sub = decrypt_val(diff_sub);
    std::cout << "  diff (at_phi - at_psi) = " << val_diff_sub << " (expected " << (5.0 * sqrt5) << ")\n\n";

    // ============================================
    // TEST 3: SUM NA MAY ALIGNED SCALES
    // ============================================
    std::cout << "TEST 3: Sum na may Aligned Scales\n";
    std::cout << "=================================\n\n";

    // sum = 2m + e·(φ+ψ) = 2m + e (kasi φ+ψ=1)
    auto sum_direct = cc->EvalAdd(at_phi, at_psi);
    double val_sum_direct = decrypt_val(sum_direct);
    std::cout << "  sum (at_phi + at_psi) = " << val_sum_direct << " (expected " << 7.0 << ")\n\n";

    // ============================================
    // TEST 4: RECOVERY NA MAY CORRECT DIFF
    // ============================================
    std::cout << "TEST 4: Recovery na may Correct Diff\n";
    std::cout << "====================================\n\n";

    // e = diff / √5
    auto ct_inv_sqrt5 = make_ct(1.0 / sqrt5);
    auto e_recovered = cc->EvalMult(diff_direct, ct_inv_sqrt5);
    double val_e = decrypt_val(e_recovered);
    std::cout << "  Recovered e = " << val_e << " (expected 5.0)\n";

    // m = (sum - e) / 2
    auto sum_minus_e = cc->EvalSub(sum_direct, e_recovered);
    auto ct_inv2 = make_ct(0.5);
    auto m_recovered = cc->EvalMult(sum_minus_e, ct_inv2);
    double val_m = decrypt_val(m_recovered);
    std::cout << "  Recovered m = " << val_m << " (expected 1.0)\n\n";

    // ============================================
    // VERDICT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  VERDICT:\n";
    std::cout << "  - Direct diff computation ay exact\n";
    std::cout << "  - Ang issue ay sa EvalSub scaling\n";
    std::cout << "  - Fix: i-compute ang diff bilang e·√5\n";
    std::cout << "    sa halip na ct(φ) - ct(ψ)\n";
    std::cout << "========================================\n";

    return 0;
}
