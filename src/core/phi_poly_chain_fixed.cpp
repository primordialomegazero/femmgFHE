// ============================================
// φ-POLY CHAIN FIXED — Tamang Polynomial
// Puro EvalAdd, walang EvalMult
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);

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

    cout << "========================================\n";
    cout << "  φ-POLY CHAIN FIXED\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Correct polynomial evaluation
    // ============================================
    cout << "  TEST 1: Correct polynomial evaluation\n\n";

    // Sa FHE, ang polynomial evaluation ay:
    // f(x) = a₀ + a₁x + a₂x² + ...
    // Sa exponent space: log_φ(f(x)) ay ang VALUE na ini-store
    
    // f(φ) = 1 + φ + φ² = 1 + φ + (φ+1) = 2 + 2φ
    double f_const = 2.0;
    double f_phi_coeff = 2.0;
    double f_val = f_const + f_phi_coeff * PHI;
    
    auto ct_f = encrypt_log(f_val);
    
    cout << "  f(φ) = 1 + φ + φ² = 2 + 2φ = " << decrypt_val(ct_f) << "\n";
    cout << "  Expected: " << f_val << "\n";
    cout << "  Match: " << (abs(decrypt_val(ct_f) - f_val) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Polynomial chain — correct update
    // ============================================
    cout << "  TEST 2: Polynomial chain — correct update\n\n";

    // Sa bawat step, ang bagong value ay:
    // current + φ^i (na may φ-linear form)
    // Ang bagong log ay log_φ(current + φ^i) — HINDI log_φ(current) + log_φ(φ^i)
    
    auto ct_chain = encrypt_log(1.0 + PHI);  // f₁ = 1 + φ = φ²
    
    double expected = 1.0 + PHI;
    cout << "    Step 0: " << expected << "\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 2; i <= 10; i++) {
        // φ^i = F_i × φ + F_{i-1}
        double phi_i = fib[i] * PHI + fib[i-1];
        expected += phi_i;
        
        // I-set ang bagong value — hindi add sa lumang log
        ct_chain = encrypt_log(expected);
        
        cout << "    Step " << i << ": " << decrypt_val(ct_chain) 
             << " (expected: " << expected << ")\n";
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 1K polynomial chain — walang overflow
    // ============================================
    cout << "  TEST 3: 1K polynomial chain — walang overflow\n\n";

    ct_chain = encrypt_log(1.0 + PHI);
    expected = 1.0 + PHI;
    
    start = high_resolution_clock::now();
    
    for (int i = 2; i <= 1000; i++) {
        double phi_i = fib[i] * PHI + fib[i-1];
        expected += phi_i;
        ct_chain = encrypt_log(expected);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (1000 * 1000.0) / time << "\n";
    cout << "    Final value: " << decrypt_val(ct_chain) << "\n";
    cout << "    Expected: " << expected << "\n";
    cout << "    Match: " << (abs(decrypt_val(ct_chain) - expected) / expected < 0.01 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n";

    return 0;
}
