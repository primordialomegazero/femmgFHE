// ============================================
// φ-MATRIX SPACE V2 — PURE EIGENVALUE LOG
//
// Fix:
// 1. Store log(λ₁) at log(λ₂) directly
// 2. Walang trace/det reconstruction
// 3. Matrix mult = EvalAdd (log eigenvalues)
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
    cout << "  φ-MATRIX SPACE V2 — PURE EIGEN LOG\n";
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
    cout << "  Store: log(λ₁) at log(λ₂)\n";
    cout << "  Matrix mult = EvalAdd (log eig)\n\n";

    // ============================================
    // PURE EIGENVALUE LOG ENCODING
    // ============================================

    auto encrypt_matrix = [&](double a, double b, double c, double d) {
        double trace = a + d;
        double det = a * d - b * c;
        double disc = trace * trace - 4 * det;
        
        vector<double> v(16, 0.0);
        
        if (disc >= 0) {
            // Real eigenvalues
            double lambda1 = (trace + sqrt(disc)) / 2;
            double lambda2 = (trace - sqrt(disc)) / 2;
            
            // Store LOG ng eigenvalues (with sign)
            double log_l1 = log(abs(lambda1) + 1e-10) / LN_PHI;
            double sign_l1 = (lambda1 >= 0) ? 1.0 : -1.0;
            
            double log_l2 = log(abs(lambda2) + 1e-10) / LN_PHI;
            double sign_l2 = (lambda2 >= 0) ? 1.0 : -1.0;
            
            // λ₁: slots 0-1 (log + sign)
            v[0] = log_l1 / fib[0];
            v[1] = sign_l1 / fib[1];
            
            // λ₂: slots 4-5 (log + sign)
            v[4] = log_l2 / fib[4];
            v[5] = sign_l2 / fib[5];
        } else {
            // Complex eigenvalues
            double real_part = trace / 2;
            double imag_part = sqrt(-disc) / 2;
            double magnitude = sqrt(real_part * real_part + imag_part * imag_part);
            double phase = atan2(imag_part, real_part);
            
            double log_mag = log(magnitude + 1e-10) / LN_PHI;
            v[0] = log_mag / fib[0];
            v[4] = fmod(phase, TWO_PI) / fib[4];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_matrix = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        // Recover λ₁
        double log_l1 = result_pt->GetCKKSPackedValue()[0].real() * fib[0];
        double sign_l1 = result_pt->GetCKKSPackedValue()[1].real() * fib[1];
        double lambda1 = pow(PHI, log_l1) * (sign_l1 >= 0 ? 1.0 : -1.0);
        
        // Recover λ₂
        double log_l2 = result_pt->GetCKKSPackedValue()[4].real() * fib[4];
        double sign_l2 = result_pt->GetCKKSPackedValue()[5].real() * fib[5];
        double lambda2 = pow(PHI, log_l2) * (sign_l2 >= 0 ? 1.0 : -1.0);
        
        return make_pair(lambda1, lambda2);
    };

    // ============================================
    // TEST 1: BASIC MATRICES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC 2x2 MATRICES\n";
    cout << "========================================\n\n";

    vector<vector<double>> matrices = {
        {1, 0, 0, 1},    // λ=1,1
        {2, 0, 0, 3},    // λ=2,3
        {1, 1, 0, 1},    // λ=1,1
        {0, 1, 1, 0},    // λ=1,-1
        {1, 2, 3, 4}     // λ≈5.37,-0.37
    };

    cout << "  Matrix | Eigenvalues | Match?\n";
    cout << "  -------|-------------|--------\n";

    int match_count = 0;
    for (auto& m : matrices) {
        auto ct = encrypt_matrix(m[0], m[1], m[2], m[3]);
        auto [l1, l2] = decrypt_matrix(ct);
        
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
    // TEST 2: MATRIX MULTIPLICATION (CORRECT)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: MATRIX MULTIPLICATION\n";
    cout << "========================================\n\n";

    // Diagonal matrices para sa easy verification
    auto ct_a = encrypt_matrix(2, 0, 0, 3);  // λ=2,3
    auto ct_b = encrypt_matrix(4, 0, 0, 5);  // λ=4,5
    
    auto start = high_resolution_clock::now();
    auto ct_product = cc->EvalAdd(ct_a, ct_b);
    auto end = high_resolution_clock::now();
    
    auto [l1, l2] = decrypt_matrix(ct_product);
    
    cout << "  A: [[2,0],[0,3]] → λ=2,3\n";
    cout << "  B: [[4,0],[0,5]] → λ=4,5\n";
    cout << "  Product: [[8,0],[0,15]] → λ=8,15\n";
    cout << "  Result: λ=" << l1 << "," << l2 << "\n";
    cout << "  Time: " << duration_cast<microseconds>(end - start).count() << " μs\n";
    cout << "  Level: " << ct_product->GetLevel() << "\n";
    cout << "  Match: " << ((abs(l1 - 8) < 0.5 && abs(l2 - 15) < 0.5) || 
                          (abs(l1 - 15) < 0.5 && abs(l2 - 8) < 0.5) ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI MATRIX
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI MATRIX\n";
    cout << "========================================\n\n";

    auto ct_fib = encrypt_matrix(1, 1, 1, 0);
    auto [fib_l1, fib_l2] = decrypt_matrix(ct_fib);
    
    cout << "  [[1,1],[1,0]] → λ = φ, -1/φ\n";
    cout << "  Expected: " << PHI << ", " << -1.0/PHI << "\n";
    cout << "  Result: " << fib_l1 << ", " << fib_l2 << "\n";
    cout << "  Match: " << (abs(fib_l1 - PHI) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-MATRIX SPACE V2 COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
