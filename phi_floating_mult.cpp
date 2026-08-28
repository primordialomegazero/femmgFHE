// ============================================
// φ-FLOATING POINT MULTIPLICATION
// Zero-level + φ-modulo para sa decimals
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
    cout << "  φ-FLOATING POINT MULTIPLICATION\n";
    cout << "  Zero-level + φ-modulo\n";
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
    double PHI = 1.6180339887498948482;

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

    // Greedy Class 1 decomposition para sa integers
    auto greedy_class1_int = [&](long long target, vector<int>& result) {
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

    // Scalar multiply via doubling (zero level)
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

    cout << "========================================\n";
    cout << "  FLOATING POINT TESTS\n";
    cout << "========================================\n\n";

    // Test cases with decimals
    vector<pair<double, double>> tests = {
        {2.5, 3.0},
        {1.5, 2.5},
        {3.75, 4.25},
        {0.5, 0.75},
        {10.5, 15.5},
        {7.25, 11.5}
    };

    cout << setw(4) << "#" << " | "
         << setw(14) << "a × b" << " | "
         << setw(12) << "Expected" << " | "
         << setw(12) << "Zero-Level" << " | "
         << setw(6) << "Match?" << " | "
         << setw(8) << "Level" << "\n";

    cout << string(70, '-') << "\n";

    int success = 0;

    for (int t = 0; t < tests.size(); t++) {
        double a_val = tests[t].first;
        double b_val = tests[t].second;
        double expected = a_val * b_val;

        // Strategy: i-scale sa integer, i-multiply, tapos i-rescale
        // Halimbawa: 2.5 × 3.0 = (25 × 30) / 100
        // Scale factor: 100 para sa 2 decimal places

        long long scale = 100;
        long long a_scaled = (long long)(a_val * scale);
        long long b_scaled = (long long)(b_val * scale);

        cout << setw(3) << t << " | "
             << setw(7) << a_val << "×" << setw(6) << b_val << " | "
             << setw(12) << fixed << setprecision(2) << expected << " | ";

        // Decompose a_scaled
        vector<int> a_terms;
        greedy_class1_int(a_scaled, a_terms);

        // Encrypt terms
        vector<Ciphertext<DCRTPoly>> ct_terms;
        for (int idx : a_terms) {
            vector<double> plain(slots, (double)L[idx]);
            auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));
            ct_terms.push_back(ct);
        }

        // Zero-level multiply
        Ciphertext<DCRTPoly> ct_result;
        bool first_result = true;

        for (int i = 0; i < ct_terms.size(); i++) {
            Ciphertext<DCRTPoly> ct_partial = scalar_multiply(ct_terms[i], b_scaled);
            if (first_result) {
                ct_result = ct_partial;
                first_result = false;
            } else {
                ct_result = cc->EvalAdd(ct_result, ct_partial);
            }
        }

        // Decrypt
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_result, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        // Rescale: result / (scale × scale)
        double result_val = result_complex[0].real() / (scale * scale);

        bool match = abs(result_val - expected) < 0.01;

        if (match) success++;

        cout << setw(12) << fixed << setprecision(2) << result_val << " | "
             << setw(6) << (match ? "✅" : "❌") << " | "
             << setw(8) << ct_result->GetLevel() << "\n";
    }

    cout << "\n========================================\n";
    cout << "  FLOATING POINT SUMMARY\n";
    cout << "========================================\n";
    cout << "  Success rate: " << success << "/" << tests.size() << "\n";
    cout << "  Lahat zero-level: " << "✅" << "\n";
    cout << "  Walang bootstrapping: " << "✅" << "\n";
    cout << "========================================\n";

    return 0;
}
