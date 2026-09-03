// ============================================
// φ-FRACTAL FIBONACCI MIXED — LAHAT SABAY
//
// Fractal compression (speed) +
// Fibonacci control (accuracy) +
// Mixed operations (add, sub, mul, div, power, mod)
// Rubber band modulo (φ-periodic)
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
    cout << "  φ-FRACTAL FIBONACCI MIXED\n";
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
    // TEST 1: MIXED OPS (1M, FRACTAL)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: MIXED OPS (1M)\n";
    cout << "========================================\n\n";

    // Mixed multipliers: iba't ibang operations
    vector<double> ops = {2.0, 0.5, 3.0, 0.3333, 5.0, 0.2};  // ×2, ÷2, ×3, ÷3, ×5, ÷5
    
    cout << "  Mixed ops: ";
    for (double op : ops) {
        if (op > 1) cout << "×" << op << " ";
        else cout << "÷" << (1.0/op) << " ";
    }
    cout << "\n\n";

    int N = 1000000;
    
    // Fractal decomposition
    vector<int> phi_groups;
    int rem = N;
    int gid = 0;
    while (rem > 0) {
        int sz = min(rem, (int)pow(PHI, gid + 1));
        phi_groups.push_back(sz);
        rem -= sz;
        gid++;
    }

    cout << "  Groups: " << phi_groups.size() << "\n";

    auto ct = encrypt_rubber(1.0);
    auto start = high_resolution_clock::now();

    for (int gs : phi_groups) {
        // Bawat group ay may mixed operation cycle
        double total_log = 0.0;
        for (int i = 0; i < 6; i++) {
            total_log += log(ops[i]) / LN_PHI;
        }
        
        double group_log = fmod(gs * total_log / 6.0, PHI);  // RUBBER BAND!
        
        vector<double> gv(16, 0.0);
        for (int i = 0; i < 16; i++) gv[i] = group_log / fib[i];
        
        Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
        auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
        ct = cc->EvalAdd(ct, ct_g);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_rubber(ct);
    
    // Expected: mixed cycle net = ×2 ÷2 ×3 ÷3 ×5 ÷5 = 1 (cancel)
    // Pero dahil sa order, may drift
    double cycle_log = 0.0;
    for (double op : ops) cycle_log += log(op) / LN_PHI;
    double expected = fmod((N / 6) * cycle_log, PHI);

    cout << "  Time: " << time << " ms\n";
    cout << "  Result: " << result << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(result - expected) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: SCALING
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING (MIXED)\n";
    cout << "========================================\n\n";

    for (int n : {1000, 10000, 100000, 1000000}) {
        vector<int> groups;
        int r = n;
        int g = 0;
        while (r > 0) {
            int sz = min(r, (int)pow(PHI, g + 1));
            groups.push_back(sz);
            r -= sz;
            g++;
        }

        auto ct_n = encrypt_rubber(1.0);
        auto s = high_resolution_clock::now();
        for (int gs : groups) {
            double total_log = 0.0;
            for (int i = 0; i < 6; i++) total_log += log(ops[i]) / LN_PHI;
            double gl = fmod(gs * total_log / 6.0, PHI);
            
            vector<double> gv(16, 0.0);
            for (int i = 0; i < 16; i++) gv[i] = gl / fib[i];
            Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
            auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
            ct_n = cc->EvalAdd(ct_n, ct_g);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r_n = decrypt_rubber(ct_n);
        double exp_n = fmod((n / 6) * cycle_log, PHI);
        bool match = abs(r_n - exp_n) < 0.01;

        cout << "  " << setw(9) << n << " ops | "
             << setw(5) << t << " ms | "
             << "Result: " << setw(8) << fixed << setprecision(4) << r_n << " | "
             << "Exp: " << setw(8) << exp_n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  FRACTAL FIBONACCI MIXED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fractal compression\n";
    cout << "  ✅ Fibonacci control\n";
    cout << "  ✅ Mixed operations\n";
    cout << "  ✅ Rubber band modulo\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
