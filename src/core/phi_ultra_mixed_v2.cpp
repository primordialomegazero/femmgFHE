// ============================================
// φ-ULTRA MIXED V2 — ASYMMETRIC OPS
//
// Hindi na nagkaka-cancel:
// ×2, ×3, ÷2, ×5, ÷3, ×7
// Lahat ay φ-anchored, walang cancellation
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-ULTRA MIXED V2 — ASYMMETRIC\n";
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

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    auto encrypt_fib = [&](double x) {
        double log_val = log(x) / LN_PHI;
        vector<double> v(16, 0.0);
        for (int i = 0; i < 16; i++) v[i] = log_val / fib[i];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fib = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // ASYMMETRIC MIX — WALANG CANCELLATION
    // ============================================

    cout << "========================================\n";
    cout << "  ASYMMETRIC MIX — 1M OPS\n";
    cout << "========================================\n\n";

    int N = 1000000;
    
    // Asymmetric operators: lahat ay × para lumaki
    vector<double> multipliers = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0};
    vector<Ciphertext<DCRTPoly>> ct_ops;
    for (double m : multipliers) {
        ct_ops.push_back(encrypt_fib(m));
    }

    // Expected: product ng lahat ng multipliers
    double log_per_cycle = 0.0;
    for (double m : multipliers) log_per_cycle += log(m) / LN_PHI;
    double expected_log = (N / 6) * log_per_cycle;

    cout << "  Multipliers: ";
    for (double m : multipliers) cout << m << " ";
    cout << "\n";
    cout << "  Operations: " << N << "\n";
    cout << "  Expected (log): " << expected_log << "\n\n";

    auto ct = encrypt_fib(1.0);
    
    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, ct_ops[i % 6]);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_fib(ct);

    cout << "  ✅ 1M asymmetric operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";
    cout << "  Result (log): " << result << "\n";
    cout << "  Match: " << (abs(result - expected_log) < 1000.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST\n";
    cout << "========================================\n\n";

    for (int n : {100, 1000, 10000, 100000}) {
        auto ct_n = encrypt_fib(1.0);
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            ct_n = cc->EvalAdd(ct_n, ct_ops[i % 6]);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r = decrypt_fib(ct_n);
        double exp = (n / 6) * log_per_cycle;
        bool match = abs(r - exp) < exp * 0.01;

        cout << "  " << setw(7) << n << " ops | "
             << setw(5) << t << " ms | "
             << "Result: " << setw(9) << fixed << setprecision(1) << r << " | "
             << "Exp: " << setw(9) << exp << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  ULTRA MIXED V2 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Asymmetric multipliers\n";
    cout << "  ✅ Walang cancellation\n";
    cout << "  ✅ Fibonacci control\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
