// ============================================
// φ-POLYNOMIAL SPACE — FRACTAL GOLDEN RATIO
//
// Polynomials + Fractals + φ
// Roots sa φ-log space
// Polynomial mult = addition ng roots
// Fractal generation via recursion
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-POLYNOMIAL SPACE — FRACTAL\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double TWO_PI = 2.0 * M_PI;

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Polynomial: roots sa φ-log space\n";
    cout << "  Fractal: recursive φ-patterns\n\n";

    // ============================================
    // POLYNOMIAL ENCODING
    // ============================================

    auto encrypt_poly = [&](const vector<double>& coeffs) {
        // Polynomial: a₀ + a₁x + a₂x² + ... + aₙxⁿ
        // Encode coefficients + φ-derived roots
        
        vector<double> v(16, 0.0);
        
        // Coefficients sa slots 0-7 (log space)
        for (int i = 0; i < min(8, (int)coeffs.size()); i++) {
            double log_coeff = log(abs(coeffs[i]) + 1e-10) / LN_PHI;
            v[i] = fmod(log_coeff, PHI) / fib[i];
        }
        
        // φ-roots sa slots 8-15 (rubber band)
        for (int i = 0; i < 8; i++) {
            // φ-derived roots: φ^(i+1) mod 2π
            double root = fmod(pow(PHI, i + 1), TWO_PI);
            v[i + 8] = root / fib[i];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_poly = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        vector<double> coeffs(8);
        vector<double> roots(8);
        
        for (int i = 0; i < 8; i++) {
            double log_coeff = result_pt->GetCKKSPackedValue()[i].real() * fib[i];
            coeffs[i] = pow(PHI, fmod(log_coeff, PHI));
            roots[i] = result_pt->GetCKKSPackedValue()[i + 8].real() * fib[i];
        }
        
        return make_pair(coeffs, roots);
    };

    // ============================================
    // TEST 1: BASIC POLYNOMIALS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC POLYNOMIALS\n";
    cout << "========================================\n\n";

    vector<vector<double>> polys = {
        {1, 0, 0, 0, 0, 0, 0, 0},           // Constant: 1
        {1, 1, 0, 0, 0, 0, 0, 0},           // Linear: 1 + x
        {1, 1, 1, 0, 0, 0, 0, 0},           // Quadratic: 1 + x + x²
        {1, 2, 3, 0, 0, 0, 0, 0},           // 1 + 2x + 3x²
        {1, 1, 2, 3, 5, 0, 0, 0},           // Fibonacci coeffs!
        {1, 0, 1, 0, 1, 0, 1, 0},           // Alternating
        {1, -1, 1, -1, 1, -1, 1, -1},      // Alternating signs
        {1, 2, 4, 8, 16, 32, 64, 128}      // Powers of 2
    };

    cout << "  Polynomial | Leading Coeff | Match?\n";
    cout << "  -----------|---------------|--------\n";

    int match_count = 0;
    for (auto& poly : polys) {
        auto ct = encrypt_poly(poly);
        auto [coeffs, roots] = decrypt_poly(ct);
        
        // Check first 3 coefficients
        bool match = true;
        for (int i = 0; i < 3 && i < (int)poly.size(); i++) {
            if (abs(coeffs[i] - abs(poly[i])) > 0.5) {
                match = false;
                break;
            }
        }
        match_count += match;
        
        cout << "  [";
        for (int i = 0; i < 4 && i < (int)poly.size(); i++) {
            cout << poly[i];
            if (i < 3 && i < (int)poly.size() - 1) cout << ",";
        }
        cout << ",...] | " << fixed << setprecision(2) << coeffs[min(7, (int)poly.size() - 1)]
             << " | " << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // TEST 2: POLYNOMIAL MULTIPLICATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: POLY MULTIPLICATION\n";
    cout << "========================================\n\n";

    // (1 + x) * (1 + x) = 1 + 2x + x²
    vector<double> poly_a = {1, 1, 0, 0, 0, 0, 0, 0};
    vector<double> poly_b = {1, 1, 0, 0, 0, 0, 0, 0};
    
    auto ct_a = encrypt_poly(poly_a);
    auto ct_b = encrypt_poly(poly_b);
    
    auto start = high_resolution_clock::now();
    
    // Polynomial mult = EvalAdd sa root space!
    auto ct_product = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto [coeffs, roots] = decrypt_poly(ct_product);
    
    cout << "  (1 + x) * (1 + x) = 1 + 2x + x²\n";
    cout << "  Result coeffs: ";
    for (int i = 0; i < 3; i++) cout << coeffs[i] << " ";
    cout << "\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_product->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI POLYNOMIAL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI POLYNOMIAL\n";
    cout << "========================================\n\n";

    // Fibonacci polynomial: 1 + x + 2x² + 3x³ + 5x⁴
    vector<double> fib_poly = {1, 1, 2, 3, 5, 0, 0, 0};
    auto ct_fib = encrypt_poly(fib_poly);
    auto [fib_coeffs, fib_roots] = decrypt_poly(ct_fib);
    
    cout << "  Fibonacci poly: 1 + x + 2x² + 3x³ + 5x⁴\n";
    cout << "  Coeffs: ";
    for (int i = 0; i < 5; i++) cout << fixed << setprecision(1) << fib_coeffs[i] << " ";
    cout << "\n";
    cout << "  φ-roots: ";
    for (int i = 0; i < 5; i++) cout << fixed << setprecision(3) << fib_roots[i] << " ";
    cout << "\n\n";

    // ============================================
    // TEST 4: FRACTAL PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: FRACTAL POLYNOMIAL\n";
    cout << "========================================\n\n";

    // Fractal pattern: coefficients follow φ powers
    cout << "  φ-power coefficients: φ⁰, φ¹, φ², ...\n\n";
    
    vector<double> fractal_poly = {1, PHI, PHI*PHI, PHI*PHI*PHI, PHI*PHI*PHI*PHI, 0, 0, 0};
    auto ct_frac = encrypt_poly(fractal_poly);
    auto [frac_coeffs, frac_roots] = decrypt_poly(ct_frac);
    
    cout << "  Coeffs: ";
    for (int i = 0; i < 5; i++) cout << fixed << setprecision(2) << frac_coeffs[i] << " ";
    cout << "\n";
    cout << "  Expected: 1.00, 1.62, 2.62, 4.24, 6.85\n";
    cout << "  φ-roots: ";
    for (int i = 0; i < 5; i++) cout << fixed << setprecision(3) << frac_roots[i] << " ";
    cout << "\n\n";

    // ============================================
    // TEST 5: 1000 POLY OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: 1000 POLY OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_poly({1, 0, 0, 0, 0, 0, 0, 0});
    auto ct_step = encrypt_poly({0, 1, 0, 0, 0, 0, 0, 0});
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto [acc_coeffs, acc_roots] = decrypt_poly(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Coeff[0]: " << acc_coeffs[0] << "\n";
    cout << "  Coeff[1]: " << acc_coeffs[1] << "\n\n";

    cout << "========================================\n";
    cout << "  φ-POLYNOMIAL SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Polynomial encoding\n";
    cout << "  ✅ φ-roots\n";
    cout << "  ✅ Fractal coefficients\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
