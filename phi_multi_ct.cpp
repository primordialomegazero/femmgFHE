// ============================================
// φ-MULTI-CIPHERTEXT APPROACH
// Walang SIMD — bawat term ay separate ct
// Lahat zero-level, walang masking
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
    cout << "  φ-MULTI-CIPHERTEXT APPROACH\n";
    cout << "  Walang SIMD — separate ct bawat term\n";
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

    int slots = 1; // Hindi na kailangan ng SIMD

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

    // Greedy Class 1 decomposition
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
    cout << "  PER-SLOT SIMULATION (NON-SIMD)\n";
    cout << "========================================\n\n";

    // Test: 8 iba't ibang multiplications
    vector<double> a_values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    vector<double> b_values = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0};

    cout << "a = [";
    for (int i = 0; i < 8; i++) cout << (i > 0 ? ", " : "") << a_values[i];
    cout << "]\n";

    cout << "b = [";
    for (int i = 0; i < 8; i++) cout << (i > 0 ? ", " : "") << b_values[i];
    cout << "]\n\n";

    cout << "Non-SIMD Approach:\n";
    cout << "Bawat multiplication ay separate ct\n";
    cout << "Walang masking, walang slot extraction\n\n";

    cout << setw(4) << "#" << " | "
         << setw(10) << "a × b" << " | "
         << setw(10) << "Expected" << " | "
         << setw(10) << "Result" << " | "
         << setw(6) << "Match?" << " | "
         << setw(8) << "Terms" << " | "
         << setw(8) << "Level" << "\n";

    cout << string(70, '-') << "\n";

    int success = 0;

    for (int i = 0; i < 8; i++) {
        long long a_val = (long long)a_values[i];
        long long b_val = (long long)b_values[i];
        double expected = a_values[i] * b_values[i];

        // Decompose a
        vector<int> a_terms;
        greedy_class1(a_val, a_terms);

        // Encrypt each term separately (non-SIMD)
        vector<Ciphertext<DCRTPoly>> ct_terms;
        for (int idx : a_terms) {
            vector<double> plain(1, (double)L[idx]);
            auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));
            ct_terms.push_back(ct);
        }

        // Zero-level multiply
        Ciphertext<DCRTPoly> ct_result;
        bool first_result = true;

        for (int j = 0; j < ct_terms.size(); j++) {
            Ciphertext<DCRTPoly> ct_partial = scalar_multiply(ct_terms[j], b_val);
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
        plain_result->SetLength(1);
        auto result_complex = plain_result->GetCKKSPackedValue();

        double result = result_complex[0].real();
        bool match = abs(result - expected) < 0.1;

        if (match) success++;

        cout << setw(3) << i << " | "
             << setw(5) << a_val << "×" << setw(4) << b_val << " | "
             << setw(10) << fixed << setprecision(0) << expected << " | "
             << setw(10) << fixed << setprecision(0) << result << " | "
             << setw(6) << (match ? "✅" : "❌") << " | "
             << setw(8) << a_terms.size() << " | "
             << setw(8) << ct_result->GetLevel() << "\n";
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Success rate: " << success << "/8\n";
    cout << "  Lahat zero-level: ✅\n";
    cout << "  Walang masking: ✅\n";
    cout << "  Walang bootstrapping: ✅\n";
    cout << "========================================\n";

    return 0;
}
