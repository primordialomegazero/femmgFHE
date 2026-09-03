// ============================================
// φ-TRUE 10K — WALANG COMPRESSION
//
// 10K tunay na EvalAdd sa encrypted domain
// Mixed operations: ×2, ÷2, ×3, ÷3, ×5, ÷5
// Fibonacci + Rubber band modulo
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
    cout << "  φ-TRUE 10K — WALANG COMPRESSION\n";
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

    // ============================================
    // RUBBER BAND ENCODING
    // ============================================

    auto encrypt_rubber = [&](double x) {
        double log_val = fmod(log(x) / LN_PHI, PHI);
        vector<double> v(16, 0.0);
        for (int i = 0; i < 16; i++) v[i] = log_val / fib[i];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_rubber = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double val = result_pt->GetCKKSPackedValue()[0].real();
        return fmod(val, PHI);
    };

    // ============================================
    // TRUE 10K MIXED — WALANG COMPRESSION
    // ============================================

    cout << "========================================\n";
    cout << "  TRUE 10K MIXED OPS\n";
    cout << "========================================\n\n";

    // Mixed operators: ×2, ÷2, ×3, ÷3, ×5, ÷5
    vector<double> ops = {2.0, 0.5, 3.0, 1.0/3.0, 5.0, 0.2};
    
    // Pre-encrypt ang bawat operator
    vector<Ciphertext<DCRTPoly>> ct_ops;
    for (double op : ops) {
        ct_ops.push_back(encrypt_rubber(op));
    }

    cout << "  Mixed ops: ";
    for (double op : ops) {
        if (op > 1) cout << "×" << op << " ";
        else cout << "÷" << (1.0/op) << " ";
    }
    cout << "\n\n";

    int N = 10000;

    cout << "  Operations: " << N << " EvalAdd\n";
    cout << "  Walang compression — server nagco-compute talaga\n\n";

    // Start sa 1.0
    auto ct = encrypt_rubber(1.0);

    auto start = high_resolution_clock::now();

    // TUNAY NA EVALADD — isa-isa!
    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, ct_ops[i % 6]);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_rubber(ct);

    // Expected: net cycle = ×2 ÷2 ×3 ÷3 ×5 ÷5 = 1 (cancel)
    double cycle_log = 0.0;
    for (double op : ops) cycle_log += log(op) / LN_PHI;
    double expected = fmod((N / 6) * cycle_log, PHI);

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";
    cout << "  Towers: " << ct->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Result: " << result << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(result - expected) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SCALING TEST (TRUE)
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING (TRUE, WALANG COMPRESSION)\n";
    cout << "========================================\n\n";

    for (int n : {100, 500, 1000, 2000, 5000, 10000}) {
        auto ct_n = encrypt_rubber(1.0);
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            ct_n = cc->EvalAdd(ct_n, ct_ops[i % 6]);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r_n = decrypt_rubber(ct_n);
        double exp_n = fmod((n / 6) * cycle_log, PHI);
        bool match = abs(r_n - exp_n) < 0.01;

        cout << "  " << setw(5) << n << " ops | "
             << setw(5) << t << " ms | "
             << "Result: " << setw(8) << fixed << setprecision(4) << r_n << " | "
             << "Exp: " << setw(8) << exp_n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  TRUE 10K COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Walang compression\n";
    cout << "  ✅ Tunay na EvalAdd (isa-isa)\n";
    cout << "  ✅ Mixed operations\n";
    cout << "  ✅ Rubber band modulo\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
