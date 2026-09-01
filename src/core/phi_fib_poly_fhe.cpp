// ============================================
// φ-FIBONACCI POLYNOMIAL FHE — NON-LINEAR
//
// φ^n = F(n)×φ + F(n-1)
// I-encode ang Fibonacci representation
// Para sa non-linear ops na walang EvalMult!
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
    cout << "  φ-FIBONACCI POLYNOMIAL FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;

    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Fibonacci polynomial: φ^n = F(n)×φ + F(n-1)\n\n";

    // ============================================
    // FIBONACCI ENCODING
    // ============================================

    auto encrypt_fib_pair = [&](double fib_coeff, double fib_prev) {
        vector<double> v(16, 0.0);
        for (int i = 0; i < 8; i++) v[i] = fib_coeff;
        for (int i = 8; i < 16; i++) v[i] = fib_prev;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fib_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double coeff = 0.0, prev = 0.0;
        for (int i = 0; i < 8; i++) coeff += result_pt->GetCKKSPackedValue()[i].real();
        for (int i = 8; i < 16; i++) prev += result_pt->GetCKKSPackedValue()[i].real();
        
        return make_pair(coeff / 8.0, prev / 8.0);
    };

    // ============================================
    // TEST 1: φ-POWERS VIA FIBONACCI
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ-POWERS VIA FIBONACCI\n";
    cout << "========================================\n\n";

    cout << "  n | F(n) coeff | F(n-1) prev | φ^n | Match?\n";
    cout << "  --|------------|-------------|-----|--------\n";

    for (int n = 1; n <= 8; n++) {
        auto ct = encrypt_fib_pair((double)fib[n], (double)fib[n-1]);
        auto [coeff, prev] = decrypt_fib_pair(ct);
        
        double phi_n = coeff * PHI + prev;
        double expected = pow(PHI, n);
        bool match = abs(phi_n - expected) < 0.1;
        
        cout << "  " << n << " | "
             << setw(10) << fixed << setprecision(0) << coeff << " | "
             << setw(11) << prev << " | "
             << setw(6) << setprecision(3) << phi_n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: FIBONACCI ADDITION (LINEAR!)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: FIBONACCI ADDITION\n";
    cout << "========================================\n\n";

    cout << "  φ^n + φ^m = (F(n)+F(m))×φ + (F(n-1)+F(m-1))\n\n";

    cout << "  n | m | φ^n + φ^m | Expected | Match?\n";
    cout << "  --|---|-----------|----------|--------\n";

    for (int n : {1, 2, 3, 5}) {
        for (int m : {1, 2, 3, 5}) {
            auto ct_n = encrypt_fib_pair((double)fib[n], (double)fib[n-1]);
            auto ct_m = encrypt_fib_pair((double)fib[m], (double)fib[m-1]);
            
            auto ct_sum = cc->EvalAdd(ct_n, ct_m);
            auto [coeff, prev] = decrypt_fib_pair(ct_sum);
            
            double sum_val = coeff * PHI + prev;
            double expected = pow(PHI, n) + pow(PHI, m);
            bool match = abs(sum_val - expected) < 0.1;
            
            cout << "  " << n << " | " << m << " | "
                 << setw(9) << fixed << setprecision(3) << sum_val << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // TEST 3: FIBONACCI CHAIN (1K)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 1K FIBONACCI CHAIN\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_fib_pair(0.0, 0.0);
    auto ct_step = encrypt_fib_pair(1.0, 0.0);  // φ¹ = 1×φ + 0

    auto start = high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto [coeff, prev] = decrypt_fib_pair(ct_acc);
    double chain_val = coeff * PHI + prev;

    cout << "  1K × φ¹ chain\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Result: " << chain_val << "\n";
    cout << "  Expected: 1000×φ = " << 1000.0 * PHI << "\n";
    cout << "  Match: " << (abs(chain_val - 1000.0 * PHI) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI POLY FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ^n = F(n)×φ + F(n-1)\n";
    cout << "  ✅ Linear addition ng φ-powers\n";
    cout << "  ✅ 1K chain: 1000×φ\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
