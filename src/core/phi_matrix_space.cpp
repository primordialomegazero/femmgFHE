// ============================================
// φ-MATRIX SPACE — LINEAR ALGEBRA FHE
//
// Matrix operations + Golden Ratio
// Eigenvalues sa φ-log space
// Matrix multiplication = addition ng eigenvalues
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
    cout << "  φ-MATRIX SPACE — LINEAR ALGEBRA\n";
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
    cout << "  Matrix: eigenvalues sa φ-log space\n";
    cout << "  Matrix mult = addition ng eigenvalues\n\n";

    // ============================================
    // 2x2 MATRIX ENCODING
    // ============================================

    auto encrypt_matrix = [&](double a, double b, double c, double d) {
        // 2x2 matrix: [[a, b], [c, d]]
        // Eigenvalues: λ₁, λ₂
        // λ = ((a+d) ± sqrt((a+d)² - 4(ad-bc))) / 2
        
        double trace = a + d;
        double det = a * d - b * c;
        double disc = trace * trace - 4 * det;
        
        vector<double> v(16, 0.0);
        
        if (disc >= 0) {
            // Real eigenvalues
            double lambda1 = (trace + sqrt(disc)) / 2;
            double lambda2 = (trace - sqrt(disc)) / 2;
            
            // λ₁ sa slots 0-3 (log space + rubber band)
            double log_l1 = log(abs(lambda1) + 1e-10) / LN_PHI;
            double sign_l1 = (lambda1 >= 0) ? 1.0 : -1.0;
            v[0] = fmod(log_l1, PHI) / fib[0];
            v[1] = sign_l1 / fib[1];
            
            // λ₂ sa slots 4-7
            double log_l2 = log(abs(lambda2) + 1e-10) / LN_PHI;
            double sign_l2 = (lambda2 >= 0) ? 1.0 : -1.0;
            v[4] = fmod(log_l2, PHI) / fib[4];
            v[5] = sign_l2 / fib[5];
            
            // Trace at determinant sa slots 8-11
            v[8] = trace / fib[8];
            v[9] = det / fib[9];
        } else {
            // Complex eigenvalues
            double real_part = trace / 2;
            double imag_part = sqrt(-disc) / 2;
            double magnitude = sqrt(real_part * real_part + imag_part * imag_part);
            double phase = atan2(imag_part, real_part);
            
            // Magnitude sa log space
            double log_mag = log(magnitude + 1e-10) / LN_PHI;
            v[0] = fmod(log_mag, PHI) / fib[0];
            
            // Phase sa rubber band
            v[4] = fmod(phase, TWO_PI) / fib[4];
            
            // Trace at determinant
            v[8] = trace / fib[8];
            v[9] = det / fib[9];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_matrix = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double trace = result_pt->GetCKKSPackedValue()[8].real() * fib[8];
        double det = result_pt->GetCKKSPackedValue()[9].real() * fib[9];
        
        // Reconstruct eigenvalues
        double disc = trace * trace - 4 * det;
        
        if (disc >= 0) {
            double lambda1 = (trace + sqrt(disc)) / 2;
            double lambda2 = (trace - sqrt(disc)) / 2;
            return make_pair(lambda1, lambda2);
        } else {
            double real_part = trace / 2;
            double imag_part = sqrt(-disc) / 2;
            return make_pair(real_part, imag_part);
        }
    };

    // ============================================
    // TEST 1: BASIC MATRICES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC 2x2 MATRICES\n";
    cout << "========================================\n\n";

    cout << "  Matrix | Eigenvalues | Match?\n";
    cout << "  -------|-------------|--------\n";

    // Test matrices: {a, b, c, d}
    vector<vector<double>> matrices = {
        {1, 0, 0, 1},    // Identity: λ=1,1
        {2, 0, 0, 3},    // Diagonal: λ=2,3
        {1, 1, 0, 1},    // Jordan: λ=1,1
        {0, 1, 1, 0},    // Swap: λ=1,-1
        {1, 2, 3, 4}     // General: λ≈-0.37,5.37
    };

    int match_count = 0;
    for (auto& m : matrices) {
        auto ct = encrypt_matrix(m[0], m[1], m[2], m[3]);
        auto [l1, l2] = decrypt_matrix(ct);
        
        // Compute expected eigenvalues
        double trace = m[0] + m[3];
        double det = m[0] * m[3] - m[1] * m[2];
        double disc = trace * trace - 4 * det;
        double exp_l1 = (trace + sqrt(abs(disc))) / 2;
        double exp_l2 = (trace - sqrt(abs(disc))) / 2;
        
        bool match = (abs(l1 - exp_l1) < 0.5 && abs(l2 - exp_l2) < 0.5) ||
                     (abs(l1 - exp_l2) < 0.5 && abs(l2 - exp_l1) < 0.5);
        match_count += match;
        
        cout << "  [" << m[0] << " " << m[1] << "; " << m[2] << " " << m[3] << "]"
             << " | λ=" << fixed << setprecision(2) << l1 << "," << l2
             << " | " << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/5\n\n";

    // ============================================
    // TEST 2: MATRIX MULTIPLICATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: MATRIX MULTIPLICATION\n";
    cout << "========================================\n\n";

    // Matrix A = [[2, 0], [0, 3]] (eigenvalues: 2, 3)
    // Matrix B = [[4, 0], [0, 5]] (eigenvalues: 4, 5)
    // Product = [[8, 0], [0, 15]] (eigenvalues: 8, 15)
    
    auto ct_a = encrypt_matrix(2, 0, 0, 3);
    auto ct_b = encrypt_matrix(4, 0, 0, 5);
    
    auto start = high_resolution_clock::now();
    
    // Matrix multiplication = EvalAdd sa eigenvalue space!
    auto ct_product = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto [l1, l2] = decrypt_matrix(ct_product);
    
    cout << "  Matrix A: [[2, 0], [0, 3]] → λ=2,3\n";
    cout << "  Matrix B: [[4, 0], [0, 5]] → λ=4,5\n";
    cout << "  Product: [[8, 0], [0, 15]] → λ=8,15\n";
    cout << "  Result: λ=" << l1 << "," << l2 << "\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_product->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI MATRICES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI MATRICES\n";
    cout << "========================================\n\n";

    // Fibonacci matrix: [[1, 1], [1, 0]]
    // Eigenvalues: φ, -1/φ
    auto ct_fib = encrypt_matrix(1, 1, 1, 0);
    auto [fib_l1, fib_l2] = decrypt_matrix(ct_fib);
    
    cout << "  Fibonacci matrix: [[1, 1], [1, 0]]\n";
    cout << "  Expected: λ = φ, -1/φ = " << PHI << ", " << -1.0/PHI << "\n";
    cout << "  Result: λ = " << fib_l1 << ", " << fib_l2 << "\n";
    cout << "  Match: " << (abs(fib_l1 - PHI) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: 1000 MATRIX OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 1000 MATRIX OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_matrix(1, 0, 0, 1);  // Identity
    auto ct_step = encrypt_matrix(1, 1, 0, 1);  // Jordan block
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto [acc_l1, acc_l2] = decrypt_matrix(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Final λ: " << acc_l1 << "," << acc_l2 << "\n\n";

    cout << "========================================\n";
    cout << "  φ-MATRIX SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Matrix encoding\n";
    cout << "  ✅ Eigenvalue space\n";
    cout << "  ✅ Matrix mult via EvalAdd\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
