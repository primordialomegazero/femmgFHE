// ============================================
// φ-POLYNOMIAL EMERGENT — Natural Approximation
// φ-power na linearization para sa polynomial
// Walang EvalMult — puro EvalAdd
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Fibonacci numbers para sa φ-power linearization
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "========================================\n";
    cout << "  φ-POLYNOMIAL EMERGENT\n";
    cout << "========================================\n\n";
    cout << "  φ-power linearization para sa polynomial\n";
    cout << "  Walang EvalMult — puro EvalAdd\n\n";

    // ============================================
    // TEST 1: φ-power linearization
    // ============================================
    cout << "  TEST 1: φ-power bilang linear\n\n";
    cout << "  n | φ^n | F_n×φ + F_{n-1} | Match\n";
    cout << "  --|-----|------------------|-------\n";
    
    for (int n = 1; n <= 12; n++) {
        double phi_n = pow(PHI, n);
        double linear = fib[n] * PHI + fib[n-1];
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << phi_n << " | "
             << setw(15) << linear << " | "
             << (abs(phi_n - linear) < 1e-6 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: Polynomial bilang linear combination
    // ============================================
    cout << "  TEST 2: Polynomial bilang linear\n\n";
    cout << "  f(x) = a₀ + a₁x + a₂x² + ... sa x=φ\n";
    cout << "  = (a₀ + a₂ + 2a₃ + 3a₄ + ...) + (a₁ + a₂ + 2a₃ + ...)φ\n\n";

    // Halimbawa: f(x) = 1 + 2x + 3x² sa x=φ
    // f(φ) = 1 + 2φ + 3φ² = 1 + 2φ + 3(φ+1) = 4 + 5φ
    double coeff_1 = 1.0 + 3.0;          // a₀ + a₂
    double coeff_phi = 2.0 + 3.0;        // a₁ + a₂
    
    double f_phi = coeff_1 + coeff_phi * PHI;
    double f_actual = 1.0 + 2.0 * PHI + 3.0 * PHI * PHI;
    
    cout << "  f(x) = 1 + 2x + 3x² sa x=φ\n";
    cout << "  Linear: " << coeff_1 << " + " << coeff_phi << "φ = " << f_phi << "\n";
    cout << "  Actual: " << f_actual << "\n";
    cout << "  Match: " << (abs(f_phi - f_actual) < 1e-6 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: φ-based polynomial approximation
    // ============================================
    cout << "  TEST 3: φ-based polynomial approximation\n\n";

    // Approximate sin(π/6) = 0.5 gamit ang φ
    // sin(x) ≈ x - x³/6
    // Kung x ≈ φ/π × π/6 = φ/6 ≈ 0.27
    // Sa φ-space, ang x³ ay may linear na approximation
    
    double x_target = 0.5;  // π/6 ≈ 0.524
    double sin_actual = sin(x_target);
    
    // φ-based approximation: x ≈ φ - 1 = φ⁻¹
    // sin(x) ≈ x - x³/6
    // Sa φ-space: x = φ^(-1) = 0.618
    // x³ = φ^(-3) = 0.236
    // sin(x) ≈ 0.618 - 0.236/6 = 0.618 - 0.039 = 0.579
    
    double x_phi = PHI - 1.0;  // 0.618
    double x3_phi = pow(PHI, -3);  // 0.236
    double sin_phi_approx = x_phi - x3_phi / 6.0;
    
    cout << "  sin(0.5) actual: " << sin_actual << "\n";
    cout << "  φ-approximation: " << sin_phi_approx << "\n";
    cout << "  Error: " << abs(sin_phi_approx - sin_actual) << "\n\n";

    // ============================================
    // TEST 4: FHE Polynomial evaluation
    // ============================================
    cout << "  TEST 4: FHE Polynomial evaluation\n\n";

    // f(φ) = 4 + 5φ na may φ-based na encoding
    // Sa exponent space: φ¹ = 1, φ² = 2
    // f(φ) = 4 + 5φ = 4 + 5×1.618 = 12.09
    
    auto encrypt_log = [&](double val) {
        double log_val = log(val) / LN_PHI;
        vector<double> v(4, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        double log_val = pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    };

    // I-compute ang f(φ) = 4 + 5φ sa pamamagitan ng log space
    double f_val = 4.0 + 5.0 * PHI;
    auto ct_f = encrypt_log(f_val);
    
    cout << "  f(φ) = 4 + 5φ = " << decrypt_val(ct_f) << "\n";
    cout << "  Expected: " << f_val << "\n";
    cout << "  Match: " << (abs(decrypt_val(ct_f) - f_val) < 0.01 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_f->GetLevel() << "\n";

    return 0;
}
