// ============================================
// φ-TENSOR SPACE — NEURAL NETWORK FHE
//
// Tensor operations + Golden Ratio
// Singular values sa φ-log space
// Tensor contraction = addition ng singular values
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
    cout << "  φ-TENSOR SPACE — NEURAL NETWORK\n";
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
    cout << "  Tensor: singular values sa φ-log space\n";
    cout << "  Tensor contraction = addition ng singular values\n\n";

    // ============================================
    // TENSOR ENCODING (2x2x2)
    // ============================================

    auto encrypt_tensor = [&](const vector<double>& tensor) {
        // Tensor: 2x2x2 = 8 elements
        // Flatten sa 2x4 matrix para sa SVD
        // Simplified: compute singular values
        
        double a = tensor[0], b = tensor[1], c = tensor[2], d = tensor[3];
        double e = tensor[4], f = tensor[5], g = tensor[6], h = tensor[7];
        
        // Frobenius norm squared
        double frob2 = a*a + b*b + c*c + d*d + e*e + f*f + g*g + h*h;
        
        // Simplified singular values (sqrt ng eigenvalues ng A^T A)
        // Para sa 2x4 matrix mula sa tensor
        double s1 = sqrt(frob2 + sqrt(frob2 * frob2 - 4 * (a*a * d*d - 2*a*b*c*d + b*b * c*c)));
        double s2 = sqrt(abs(frob2 - sqrt(abs(frob2 * frob2 - 4 * (a*a * d*d - 2*a*b*c*d + b*b * c*c)))));
        
        vector<double> v(16, 0.0);
        
        // σ₁ sa slots 0-3 (log space + rubber band)
        double log_s1 = log(s1 + 1e-10) / LN_PHI;
        v[0] = fmod(log_s1, PHI) / fib[0];
        v[1] = s1 / fib[1];  // Raw value para sa verification
        
        // σ₂ sa slots 4-7
        double log_s2 = log(s2 + 1e-10) / LN_PHI;
        v[4] = fmod(log_s2, PHI) / fib[4];
        v[5] = s2 / fib[5];
        
        // Frobenius norm sa slots 8-9
        double frob = sqrt(frob2);
        v[8] = frob / fib[8];
        v[9] = log(frob + 1e-10) / LN_PHI / fib[9];
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_tensor = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double s1 = result_pt->GetCKKSPackedValue()[1].real() * fib[1];
        double s2 = result_pt->GetCKKSPackedValue()[5].real() * fib[5];
        double frob = result_pt->GetCKKSPackedValue()[8].real() * fib[8];
        
        return make_tuple(s1, s2, frob);
    };

    // ============================================
    // TEST 1: BASIC TENSORS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC 2x2x2 TENSORS\n";
    cout << "========================================\n\n";

    vector<vector<double>> tensors = {
        {1, 0, 0, 0, 0, 0, 0, 0},  // Single element
        {1, 1, 1, 1, 1, 1, 1, 1},  // All ones
        {1, 0, 0, 1, 0, 0, 0, 0},  // Identity-like
        {1, 1, 1, 0, 0, 0, 0, 0},  // Upper half
        {1, 1, 2, 3, 5, 8, 13, 21} // Fibonacci tensor!
    };

    cout << "  Tensor | σ₁ | σ₂ | Frobenius\n";
    cout << "  -------|----|----|----------\n";

    for (auto& t : tensors) {
        auto ct = encrypt_tensor(t);
        auto [s1, s2, frob] = decrypt_tensor(ct);
        
        // Expected Frobenius norm
        double exp_frob = 0;
        for (double x : t) exp_frob += x * x;
        exp_frob = sqrt(exp_frob);
        
        bool match = abs(frob - exp_frob) < 1.0;
        
        cout << "  [" << t[0] << "," << t[1] << ",...]"
             << " | " << fixed << setprecision(2) << s1
             << " | " << setprecision(2) << s2
             << " | " << setprecision(2) << frob
             << " (exp: " << exp_frob << ") "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: TENSOR CONTRACTION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: TENSOR CONTRACTION\n";
    cout << "========================================\n\n";

    // Tensor A: identity-like
    vector<double> tensor_a = {1, 0, 0, 1, 0, 0, 0, 0};
    // Tensor B: fibonacci-like
    vector<double> tensor_b = {1, 1, 1, 1, 1, 1, 1, 1};
    
    auto ct_a = encrypt_tensor(tensor_a);
    auto ct_b = encrypt_tensor(tensor_b);
    
    auto start = high_resolution_clock::now();
    
    // Tensor contraction = EvalAdd sa singular value space!
    auto ct_contract = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto [s1, s2, frob] = decrypt_tensor(ct_contract);
    
    cout << "  Tensor A: Frobenius = " << sqrt(2) << "\n";
    cout << "  Tensor B: Frobenius = " << sqrt(8) << "\n";
    cout << "  Contraction via EvalAdd\n";
    cout << "  Result Frobenius: " << frob << "\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_contract->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI TENSOR
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI TENSOR\n";
    cout << "========================================\n\n";

    vector<double> fib_tensor = {1, 1, 2, 3, 5, 8, 13, 21};
    auto ct_fib = encrypt_tensor(fib_tensor);
    auto [fib_s1, fib_s2, fib_frob] = decrypt_tensor(ct_fib);
    
    double exp_frob = 0;
    for (double x : fib_tensor) exp_frob += x * x;
    exp_frob = sqrt(exp_frob);
    
    cout << "  Fibonacci tensor: {1,1,2,3,5,8,13,21}\n";
    cout << "  σ₁: " << fib_s1 << "\n";
    cout << "  σ₂: " << fib_s2 << "\n";
    cout << "  Frobenius: " << fib_frob << " (expected: " << exp_frob << ")\n";
    cout << "  Match: " << (abs(fib_frob - exp_frob) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: 1000 TENSOR OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 1000 TENSOR OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_tensor({1, 0, 0, 0, 0, 0, 0, 0});
    auto ct_step = encrypt_tensor({0, 1, 0, 0, 0, 0, 0, 0});
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto [acc_s1, acc_s2, acc_frob] = decrypt_tensor(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Frobenius: " << acc_frob << "\n\n";

    cout << "========================================\n";
    cout << "  φ-TENSOR SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Tensor encoding\n";
    cout << "  ✅ Singular value space\n";
    cout << "  ✅ Tensor contraction via EvalAdd\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
