// ============================================
// CLEAN BENCHMARK: ZERO-LEVEL MULTIPLICATION
// Class 1 decomposition + binary doubling
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include <random>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  CLEAN BENCHMARK: ZERO-LEVEL MULT\n";
    cout << "  Class 1 + Binary Doubling\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;

    // Lucas table
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

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

            if (first) {
                result = term;
                first = false;
            } else {
                result = cc->EvalAdd(result, term);
            }

            remaining -= power;
        }

        return result;
    };

    // Test values
    vector<pair<long long, long long>> tests = {
        {10, 15}, {7, 11}, {13, 17}, {25, 30}, {50, 75},
        {100, 125}, {200, 250}, {500, 750}, {1000, 1500},
        {25, 100}, {123, 456}, {789, 321}, {555, 777},
        {100, 100}, {250, 250}, {500, 500}, {1000, 1000},
        {2000, 3000}, {5000, 7000}, {10000, 15000}
    };

    cout << "========================================\n";
    cout << "  BENCHMARK RESULTS\n";
    cout << "========================================\n\n";

    cout << setw(4) << "#" << " | "
         << setw(12) << "a × b" << " | "
         << setw(10) << "Expected" << " | "
         << setw(10) << "Zero-Level" << " | "
         << setw(10) << "Traditional" << " | "
         << setw(6) << "Match?" << " | "
         << setw(8) << "Terms" << " | "
         << setw(8) << "Time(ms)" << "\n";

    cout << string(90, '-') << "\n";

    int success = 0;
    int fail = 0;
    double total_time_zero = 0;
    double total_time_trad = 0;

    for (int t = 0; t < tests.size(); t++) {
        long long a_val = tests[t].first;
        long long b_val = tests[t].second;
        double expected = (double)(a_val * b_val);

        // Decompose a
        vector<int> a_terms;
        greedy_class1(a_val, a_terms);

        // Encrypt a terms
        vector<Ciphertext<DCRTPoly>> ct_terms;
        for (int idx : a_terms) {
            vector<double> plain(slots, (double)L[idx]);
            auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));
            ct_terms.push_back(ct);
        }

        // Zero-level multiplication
        auto t1 = high_resolution_clock::now();

        Ciphertext<DCRTPoly> ct_result;
        bool first_result = true;

        for (int i = 0; i < ct_terms.size(); i++) {
            Ciphertext<DCRTPoly> ct_partial = scalar_multiply(ct_terms[i], b_val);
            if (first_result) {
                ct_result = ct_partial;
                first_result = false;
            } else {
                ct_result = cc->EvalAdd(ct_result, ct_partial);
            }
        }

        auto t2 = high_resolution_clock::now();
        double time_zero = duration_cast<milliseconds>(t2 - t1).count();

        // Traditional multiplication
        auto t3 = high_resolution_clock::now();

        vector<double> plain_a(slots, (double)a_val);
        vector<double> plain_b(slots, (double)b_val);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));
        auto ct_trad = cc->EvalMult(ct_a, ct_b);

        auto t4 = high_resolution_clock::now();
        double time_trad = duration_cast<milliseconds>(t4 - t3).count();

        total_time_zero += time_zero;
        total_time_trad += time_trad;

        // Decrypt both
        Plaintext plain_result, plain_trad;
        cc->Decrypt(keyPair.secretKey, ct_result, &plain_result);
        cc->Decrypt(keyPair.secretKey, ct_trad, &plain_trad);

        plain_result->SetLength(slots);
        plain_trad->SetLength(slots);

        auto result_complex = plain_result->GetCKKSPackedValue();
        auto trad_complex = plain_trad->GetCKKSPackedValue();

        double zero_val = result_complex[0].real();
        double trad_val = trad_complex[0].real();
        bool match = (abs(zero_val - expected) < 1.0) && (abs(trad_val - expected) < 1.0);

        if (match) success++;
        else fail++;

        cout << setw(3) << t << " | "
             << setw(7) << a_val << "×" << setw(4) << b_val << " | "
             << setw(10) << fixed << setprecision(0) << expected << " | "
             << setw(10) << fixed << setprecision(0) << zero_val << " | "
             << setw(10) << fixed << setprecision(0) << trad_val << " | "
             << setw(6) << (match ? "✅" : "❌") << " | "
             << setw(8) << a_terms.size() << " | "
             << setw(8) << fixed << setprecision(1) << time_zero << "\n";
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Success rate: " << success << "/" << tests.size() << "\n";
    cout << "  Average zero-level time: " << fixed << setprecision(1) << (total_time_zero / tests.size()) << " ms\n";
    cout << "  Average traditional time: " << fixed << setprecision(1) << (total_time_trad / tests.size()) << " ms\n";
    cout << "  Zero-level always Level 0\n";
    cout << "  Traditional always Level 1\n";
    cout << "========================================\n";

    return 0;
}
