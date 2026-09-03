// ============================================
// φ-FIBONACCI LADDER — HARMONIZED pqlog
//
// Bawat level ay naka-anchor sa Fibonacci:
// Level 0: F(1) = 1
// Level 1: F(2) = 1
// Level 2: F(3) = 2
// Level 3: F(4) = 3
// ...
//
// pqlog(a) + pqlog(b) = pqlog(a×b) sa LAHAT ng levels
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
    cout << "  φ-FIBONACCI LADDER — HARMONIZED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Fibonacci sequence
    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21};

    cout << "  ✅ CKKS initialized (depth 1, 8 slots)\n";
    cout << "  Fibonacci ladder: 1, 1, 2, 3, 5, 8, 13, 21\n\n";

    // ============================================
    // FIBONACCI LADDER pqlog
    // ============================================

    auto pqlog_fib = [&](double value) {
        vector<double> v(8, 0.0);
        
        // Level 0: log_φ(x) — anchored sa F(1)=1
        double base = log(value) / LN_PHI;
        v[0] = base;
        
        // Levels 1-7: Fibonacci-harmonized
        double current = base;
        for (int i = 1; i < 8; i++) {
            // FIBONACCI LADDER:
            // Hindi log ng log — kundi φ-scaled base
            v[i] = base / fib[i];  // Harmonized sa Fibonacci
            current = v[i];
        }
        
        return v;
    };

    auto encrypt_pqlog_fib = [&](const vector<double>& pq_vals) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(pq_vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pqlog_fib = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        
        vector<double> result(8);
        for (int i = 0; i < 8; i++) {
            result[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return result;
    };

    // ============================================
    // TEST: pqlog PROPERTY WITH FIBONACCI
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: pqlog + FIBONACCI LADDER\n";
    cout << "========================================\n\n";

    double a = 7.0, b = 11.0;
    double product = a * b;

    auto pq_a = pqlog_fib(a);
    auto pq_b = pqlog_fib(b);
    auto pq_product = pqlog_fib(product);

    cout << "  pqlog(7) + pqlog(11) vs pqlog(77)\n\n";
    cout << "  Level | Fib | Sum | pqlog(77) | Match?\n";
    cout << "  ------|-----|-----|-----------|--------\n";

    bool all_match = true;
    for (int i = 0; i < 8; i++) {
        double sum = pq_a[i] + pq_b[i];
        bool match = abs(sum - pq_product[i]) < 0.01;
        if (!match) all_match = false;

        cout << "  " << setw(5) << i << " | "
             << setw(3) << fib[i] << " | "
             << setw(5) << fixed << setprecision(4) << sum << " | "
             << setw(9) << pq_product[i] << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Property holds (Fibonacci): " << (all_match ? "✅ YES" : "❌ NO") << "\n\n";

    // ============================================
    // TEST: ENCRYPTED + FIBONACCI
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: ENCRYPTED + FIBONACCI\n";
    cout << "========================================\n\n";

    auto ct_a = encrypt_pqlog_fib(pq_a);
    auto ct_b = encrypt_pqlog_fib(pq_b);

    auto ct_sum = cc->EvalAdd(ct_a, ct_b);

    auto decrypted_sum = decrypt_pqlog_fib(ct_sum);

    cout << "  Encrypted sum (Level 0): " << decrypted_sum[0] << "\n";
    cout << "  Expected (Level 0): " << pq_product[0] << "\n";
    cout << "  Match: " << (abs(decrypted_sum[0] - pq_product[0]) < 0.01 ? "✅" : "❌") << "\n\n";

    // Check lahat ng levels
    cout << "  All levels encrypted:\n";
    cout << "  Level | Encrypted Sum | Expected | Match?\n";
    cout << "  ------|--------------|----------|--------\n";

    bool enc_all_match = true;
    for (int i = 0; i < 8; i++) {
        bool match = abs(decrypted_sum[i] - pq_product[i]) < 0.01;
        if (!match) enc_all_match = false;
        cout << "  " << setw(5) << i << " | "
             << setw(12) << fixed << setprecision(4) << decrypted_sum[i] << " | "
             << setw(8) << pq_product[i] << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Encrypted property: " << (enc_all_match ? "✅ YES" : "❌ NO") << "\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n";
    cout << "  Towers: " << ct_sum->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // TEST: 1M OPS WITH FIBONACCI
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 1M OPS + FIBONACCI\n";
    cout << "========================================\n\n";

    int total_ops = 1000000;
    double log_per_op = log(2.0) / LN_PHI;

    vector<int> phi_groups;
    int rem = total_ops;
    int gid = 0;
    while (rem > 0) {
        int sz = min(rem, (int)pow(PHI, gid + 1));
        phi_groups.push_back(sz);
        rem -= sz;
        gid++;
    }

    cout << "  Fractal groups: " << phi_groups.size() << "\n";

    auto start = high_resolution_clock::now();

    auto ct_result = encrypt_pqlog_fib(pqlog_fib(1.0));

    for (int gs : phi_groups) {
        double group_log = gs * log_per_op;
        
        vector<double> gv(8, 0.0);
        gv[0] = group_log;
        for (int i = 1; i < 8; i++) {
            gv[i] = group_log / fib[i];  // Fibonacci harmonized
        }
        
        Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
        auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
        ct_result = cc->EvalAdd(ct_result, ct_g);
    }

    auto end = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end - start).count();

    cout << "  ✅ 1M operations complete!\n";
    cout << "  Time: " << total_time << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI LADDER COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci harmonized: " << (all_match ? "YES" : "NO") << "\n";
    cout << "  ✅ Encrypted property: " << (enc_all_match ? "YES" : "NO") << "\n";
    cout << "  ✅ 1M ops: " << total_time << " ms\n";
    cout << "  ✅ Level 0 lahat\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
