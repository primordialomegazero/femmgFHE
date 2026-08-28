// ============================================
// φ-BENCHMARK — BILIS NG ZERO-LEVEL vs TRAD
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-BENCHMARK — SPEED COMPARISON\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 1;

    // Lucas table
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) L.push_back(L[i-1] + L[i-2]);

    // Class 1
    vector<long long> class1;
    vector<int> class1_idx;
    for (int i = 0; i <= 50; i++) {
        if (i % 3 == 1) {
            class1.push_back(L[i]);
            class1_idx.push_back(i);
        }
    }

    auto greedy_class1 = [&](long long target, vector<int>& result) {
        long long rem = target;
        result.clear();
        while (rem > 0) {
            int best_pos = -1;
            long long best_val = 0;
            for (int i = 0; i < class1.size(); i++) {
                if (class1[i] <= rem && class1[i] > best_val) {
                    best_val = class1[i];
                    best_pos = i;
                }
            }
            if (best_pos == -1) break;
            result.push_back(class1_idx[best_pos]);
            rem -= best_val;
        }
        return rem;
    };

    auto scalar_multiply = [&](Ciphertext<DCRTPoly> ct, long long scalar) {
        Ciphertext<DCRTPoly> result;
        bool first = true;
        long long remaining = scalar;
        while (remaining > 0) {
            long long power = 1;
            while (power * 2 <= remaining) power *= 2;
            Ciphertext<DCRTPoly> term = ct;
            long long p = power;
            while (p > 1) {
                term = cc->EvalAdd(term, term);
                p /= 2;
            }
            if (first) { result = term; first = false; }
            else { result = cc->EvalAdd(result, term); }
            remaining -= power;
        }
        return result;
    };

    // Test values
    vector<pair<long long, long long>> tests = {
        {7, 11}, {13, 17}, {25, 30}, {50, 75}, {100, 125}
    };

    cout << "TEST: 5 MULTIPLICATIONS\n\n";
    cout << setw(12) << "a × b" << " | "
         << setw(12) << "Traditional" << " | "
         << setw(12) << "Zero-Level" << " | "
         << setw(8) << "Ratio" << " | "
         << setw(8) << "Level" << "\n";
    cout << string(65, '-') << "\n";

    double total_trad = 0, total_zero = 0;

    for (auto& test : tests) {
        long long a_val = test.first;
        long long b_val = test.second;

        // Encrypt a
        vector<double> plain_a(slots, (double)a_val);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));

        // Traditional multiply
        auto t1 = high_resolution_clock::now();
        vector<double> plain_b(slots, (double)b_val);
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));
        auto ct_trad = cc->EvalMult(ct_a, ct_b);
        auto t2 = high_resolution_clock::now();
        double time_trad = duration_cast<milliseconds>(t2 - t1).count();

        // Zero-level multiply
        auto t3 = high_resolution_clock::now();
        vector<int> terms;
        greedy_class1(a_val, terms);
        Ciphertext<DCRTPoly> ct_result;
        bool first = true;
        for (int idx : terms) {
            vector<double> plain_term(slots, (double)L[idx]);
            auto ct_term = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_term));
            auto ct_partial = scalar_multiply(ct_term, b_val);
            if (first) { ct_result = ct_partial; first = false; }
            else { ct_result = cc->EvalAdd(ct_result, ct_partial); }
        }
        auto t4 = high_resolution_clock::now();
        double time_zero = duration_cast<milliseconds>(t4 - t3).count();

        total_trad += time_trad;
        total_zero += time_zero;

        cout << setw(7) << a_val << "×" << setw(4) << b_val << " | "
             << setw(8) << fixed << setprecision(1) << time_trad << " ms | "
             << setw(8) << fixed << setprecision(1) << time_zero << " ms | "
             << setw(7) << fixed << setprecision(1) << (time_zero/time_trad) << "× | "
             << setw(8) << ct_result->GetLevel() << "\n";
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Total traditional: " << fixed << setprecision(1) << total_trad << " ms\n";
    cout << "  Total zero-level: " << fixed << setprecision(1) << total_zero << " ms\n";
    cout << "  Average ratio: " << fixed << setprecision(1) << (total_zero / total_trad) << "×\n";
    cout << "========================================\n";

    return 0;
}
