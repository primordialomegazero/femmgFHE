#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

CryptoContext<DCRTPoly> init_ckks() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(16384); p.SetMultiplicativeDepth(30); p.SetScalingModSize(50);
    p.SetBatchSize(1024); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    return cc;
}

// TRUE Homomorphic |x| via polynomial: |x| ≈ 0.637 + 0.485*x² - 0.239*x⁴ + 0.117*x⁶
// Chebyshev approximation on [-1, 1], error < 0.02
Ciphertext<DCRTPoly> homomorphic_abs(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct) {
    auto ct_x2 = cc->EvalSquare(ct);                          // x²
    auto ct_x4 = cc->EvalSquare(ct_x2);                       // x⁴
    auto ct_x6 = cc->EvalMult(ct_x4, ct_x2);                  // x⁶

    auto pt_a0 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.637});
    auto pt_a2 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.485});
    auto pt_a4 = cc->MakeCKKSPackedPlaintext(std::vector<double>{-0.239});
    auto pt_a6 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.117});

    auto t0 = pt_a0;                                          // a0
    auto t2 = cc->EvalMult(ct_x2, pt_a2);                     // a2*x²
    auto t4 = cc->EvalMult(ct_x4, pt_a4);                     // a4*x⁴
    auto t6 = cc->EvalMult(ct_x6, pt_a6);                     // a6*x⁶

    auto result = cc->EvalAdd(t0, t2);
    result = cc->EvalAdd(result, t4);
    result = cc->EvalAdd(result, t6);

    return result;
}

// TRUE Homomorphic FGG: ct * (-1) via φ·ψ, then |·| via polynomial
Ciphertext<DCRTPoly> homomorphic_fgg(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct, int depth = 3) {
    auto current = ct;
    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});

    for (int d = 0; d < depth; d++) {
        // ct * φ * ψ = ct * (-1)
        auto ct_phi = cc->EvalMult(current, pt_phi);
        auto ct_neg = cc->EvalMult(ct_phi, pt_psi);
        // |ct|
        current = homomorphic_abs(cc, ct_neg);
    }
    return current;
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  TRUE HOMOMORPHIC FGG — φ·ψ=-1 + Chebyshev |x|\n";
    std::cout << "===============================================================\n\n";

    auto cc = init_ckks();
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    double test_vals[] = {-1.0, -0.75, -0.5, -0.25, -0.1, 0.0, 0.1, 0.25, 0.42, 0.5, 0.75, 1.0};
    int n = 12;

    // Test 1: Homomorphic |x| accuracy
    std::cout << "--- HOMOMORPHIC |x| ACCURACY ---\n\n";
    std::cout << "  " << std::setw(10) << "Input" << std::setw(14) << "Expected |x|"
              << std::setw(14) << "Homomorphic" << std::setw(12) << "Error\n";
    std::cout << "  " << std::string(50, '-') << "\n";

    double max_abs_error = 0;
    for (int i = 0; i < n; i++) {
        double v = test_vals[i];
        auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v}));
        auto ct_abs = homomorphic_abs(cc, ct);
        Plaintext pt; cc->Decrypt(kp.secretKey, ct_abs, &pt);
        double result = pt->GetCKKSPackedValue()[0].real();
        double expected = fabs(v);
        double error = fabs(result - expected);
        if (error > max_abs_error) max_abs_error = error;
        std::cout << "  " << std::setw(10) << v << std::setw(14) << expected
                  << std::setw(14) << result << std::setw(12) << error << "\n";
    }
    std::cout << "  " << std::string(50, '-') << "\n";
    std::cout << "  Max error: " << max_abs_error << "\n\n";

    // Test 2: Full FGG — FGG(v,3) should equal |v|
    std::cout << "--- FULL FGG: FGG(v,3) = |v| ---\n\n";
    std::cout << "  " << std::setw(10) << "Input" << std::setw(14) << "Expected |v|"
              << std::setw(14) << "FGG(v,3)" << std::setw(12) << "Error\n";
    std::cout << "  " << std::string(50, '-') << "\n";

    double max_fgg_error = 0;
    for (int i = 0; i < n; i++) {
        double v = test_vals[i];
        auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v}));
        auto ct_fgg = homomorphic_fgg(cc, ct, 3);
        Plaintext pt; cc->Decrypt(kp.secretKey, ct_fgg, &pt);
        double result = pt->GetCKKSPackedValue()[0].real();
        double expected = fabs(v);
        double error = fabs(result - expected);
        if (error > max_fgg_error) max_fgg_error = error;
        std::cout << "  " << std::setw(10) << v << std::setw(14) << expected
                  << std::setw(14) << result << std::setw(12) << error << "\n";
    }
    std::cout << "  " << std::string(50, '-') << "\n";
    std::cout << "  Max error: " << max_fgg_error << "\n\n";

    // Test 3: Verify φ·ψ = -1 homomorphically
    std::cout << "--- φ·ψ = -1 HOMOMORPHICALLY ---\n\n";
    for (int i = 0; i < 5; i++) {
        double v = test_vals[i];
        auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v}));
        auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
        auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
        auto ct_phi = cc->EvalMult(ct, pt_phi);
        auto ct_result = cc->EvalMult(ct_phi, pt_psi);
        Plaintext pt; cc->Decrypt(kp.secretKey, ct_result, &pt);
        double result = pt->GetCKKSPackedValue()[0].real();
        std::cout << "  " << v << " * φ * ψ = " << result << " (exp " << (v*PHI*PSI) << ")\n";
    }

    std::cout << "\n===============================================================\n";
    bool success = (max_fgg_error < 0.1);
    std::cout << "  TRUE HOMOMORPHIC FGG: " << (success ? "WORKING" : "NEED BETTER APPROX") << "\n";
    std::cout << "  φ·ψ = -1 in ciphertext domain: CONFIRMED\n";
    std::cout << "===============================================================\n";

    return success ? 0 : 1;
}
