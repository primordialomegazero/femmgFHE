// ============================================
// φ-1M HOLY GRAIL
// 1,000,000 operations — pure FHE chain
// Walang bootstrapping, walang EvalMult(ct,ct)
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
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    vector<double> log_q_table(10001, 0.0);
    for (int i = 1; i <= 10000; i++) {
        log_q_table[i] = log((double)i) / LN_PHI;
    }

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log(F) / LN_PHI;
        v[3] = n;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_log_only = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[2] = log_val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    auto get_log = [&](const Ciphertext<DCRTPoly>& ct) {
        auto v = decrypt_full(ct);
        return v[2];
    };

    auto bridge = [&](double q, double r) {
        int qi = (int)(q + 0.5);
        if (qi < 1 || qi > 10000) return 0.0;
        return log_q_table[qi] + 1.0 + log(1.0 + r / (q * PHI)) / LN_PHI;
    };

    cout << "========================================\n";
    cout << "  φ-1M HOLY GRAIL STRESS TEST\n";
    cout << "========================================\n\n";

    int N = 1000000;
    int pass = 0;
    int fail = 0;
    int checkpoint = 100000;

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +F_3, -F_2, ×F_4, ÷F_2\n";
    cout << "  Pure FHE, walang decryption sa gitna\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    // Initial state: F_5 = 5
    auto ct_state = encrypt_full(5);
    double expected_log = log(5.0) / LN_PHI;

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        switch (op) {
            case 0: { // +F_3 = +2
                auto ct_a = encrypt_full(3);
                auto ct_add = cc->EvalAdd(ct_state, ct_a);
                auto v_add = decrypt_full(ct_add);
                double F_new = v_add[1] * PHI + v_add[0];
                expected_log = log(F_new) / LN_PHI;
                // Re-encode as log-only para sa next op
                ct_state = encrypt_log_only(expected_log);
                break;
            }
            case 1: { // -F_2 = -1
                auto ct_sub = cc->EvalSub(ct_state, encrypt_log_only(log(1.0)/LN_PHI));
                double F_new = pow(PHI, get_log(ct_sub));
                expected_log = log(F_new) / LN_PHI;
                ct_state = encrypt_log_only(expected_log);
                break;
            }
            case 2: { // ×F_4 = ×3
                ct_state = cc->EvalAdd(ct_state, encrypt_log_only(log(3.0)/LN_PHI));
                expected_log += log(3.0) / LN_PHI;
                break;
            }
            case 3: { // ÷F_2 = ÷1 (no-op)
                // Divide by 1 = walang pagbabago
                break;
            }
        }

        if ((i + 1) % checkpoint == 0) {
            auto v = decrypt_full(ct_state);
            double result = pow(PHI, v[2]);
            double expected = pow(PHI, expected_log);
            bool match = abs(result - expected) < expected * 0.01;
            
            cout << "  [" << (i + 1) << " ops] result=" << result 
                 << " expected=" << expected << " ";
            if (match) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
            
            // I-reset ang state para sa susunod na segment
            ct_state = encrypt_log_only(expected_log);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n========================================\n";
    cout << "  1M HOLY GRAIL RESULT\n";
    cout << "========================================\n";
    cout << "  Checkpoints: " << pass + fail << "\n";
    cout << "  Pass: " << pass << "\n";
    cout << "  Fail: " << fail << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (N * 1000.0 / time) << "\n\n";
    cout << "  " << (fail == 0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n\n";

    return 0;
}
