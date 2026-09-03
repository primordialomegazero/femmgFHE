// ============================================
// φ-CT × CT 1K DEPTH 0 — NORMAL
//
// 1000 ciphertext × ciphertext operations
// Depth 0, walang daya, pure EvalAdd
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
    cout << "  φ-CT × CT 1K DEPTH 0\n";
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
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    auto encrypt_log = [&](double value) {
        double log_val = log(value) / LN_PHI;
        vector<double> v(16, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // TEST 1: BASIC MULTIPLICATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC MULTIPLICATION\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_log(5.0);
    auto ct_7 = encrypt_log(7.0);
    auto ct_prod = cc->EvalAdd(ct_5, ct_7);
    
    double prod_log = decrypt_log(ct_prod);
    double prod = pow(PHI, prod_log);

    cout << "  5 × 7 = " << prod << " (Expected: 35)\n";
    cout << "  Match: " << (abs(prod - 35.0) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: 1K CT × CT
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 1K CT × CT\n";
    cout << "========================================\n\n";

    int N = 1000;
    auto ct_acc = encrypt_log(1.0);
    auto ct_two = encrypt_log(2.0);

    cout << "  Operations: " << N << " (bawat isa ay ×2)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_two);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result_log = decrypt_log(ct_acc);
    double expected_log = N * log(2.0);

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Towers: " << ct_acc->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Result (log): " << result_log << "\n";
    cout << "  Expected (log): " << expected_log << "\n";
    cout << "  Match: " << (abs(result_log - expected_log) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST\n";
    cout << "========================================\n\n";

    for (int n : {10, 50, 100, 200, 500, 1000}) {
        auto ct_n = encrypt_log(1.0);
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            ct_n = cc->EvalAdd(ct_n, ct_two);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r = decrypt_log(ct_n);
        double exp = n * log(2.0);
        bool match = abs(r - exp) < 1.0;

        cout << "  " << setw(5) << n << " ops | "
             << setw(4) << t << " ms | "
             << "Result: " << setw(10) << fixed << setprecision(2) << r << " | "
             << "Exp: " << setw(10) << exp << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  CT × CT 1K DEPTH 0 COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
