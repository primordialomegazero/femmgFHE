// ============================================
// DEBUG: CLASS 1 MULTIPLICATION
// Alamin kung saan nag-fail ang V3
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
    cout << "  DEBUG: CLASS 1 MULTIPLICATION\n";
    cout << "  Alamin kung saan nag-fail\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(20);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;

    // Lucas table L_0 to L_50
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Class 1: L_1, L_4, L_7, L_10, ...
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

    // Helper: Multiply ct by scalar via doubling (zero level)
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

    // Test cases
    vector<pair<double, double>> tests = {
        {10, 15},
        {7, 11},
        {13, 17},
        {25, 30},
        {50, 75},
        {100, 125}
    };

    cout << "DEBUGGING BATCH TEST:\n\n";

    for (auto& test : tests) {
        double a_val = test.first;
        double b_val = test.second;
        double expected = a_val * b_val;

        cout << "----------------------------------------\n";
        cout << "TEST: " << a_val << " × " << b_val << " = " << expected << "\n\n";

        // Decompose a
        vector<int> a_terms;
        long long rem_a = greedy_class1((long long)a_val, a_terms);

        cout << "  Decomposition ng " << a_val << ": ";
        for (int idx : a_terms) cout << L[idx] << " ";
        cout << "(rem=" << rem_a << ")\n";

        // Verify decomposition
        long long sum_check = 0;
        for (int idx : a_terms) sum_check += L[idx];
        cout << "  Sum check: " << sum_check << " (expected " << a_val << ")\n";

        // Encrypt each term
        vector<Ciphertext<DCRTPoly>> ct_terms;
        for (int idx : a_terms) {
            vector<double> plain(slots, (double)L[idx]);
            auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));
            ct_terms.push_back(ct);
        }

        cout << "  Encrypted " << ct_terms.size() << " terms\n";

        // Multiply each term by b_val via scalar_multiply
        Ciphertext<DCRTPoly> ct_result;
        bool first_result = true;

        for (int i = 0; i < ct_terms.size(); i++) {
            Ciphertext<DCRTPoly> ct_partial;
            try {
                ct_partial = scalar_multiply(ct_terms[i], (long long)b_val);
            } catch (const exception& e) {
                cout << "  ERROR sa scalar_multiply: " << e.what() << "\n";
                continue;
            }

            // Decrypt partial to check
            Plaintext plain_partial;
            cc->Decrypt(keyPair.secretKey, ct_partial, &plain_partial);
            plain_partial->SetLength(slots);
            auto partial_complex = plain_partial->GetCKKSPackedValue();
            double partial_val = partial_complex[0].real();

            cout << "  Partial (" << L[a_terms[i]] << " × " << b_val << ") = " 
                 << partial_val << " (expected " << (L[a_terms[i]] * b_val) << ")\n";

            if (first_result) {
                ct_result = ct_partial;
                first_result = false;
            } else {
                ct_result = cc->EvalAdd(ct_result, ct_partial);
            }
        }

        // Decrypt result
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_result, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << "  RESULT: " << result_complex[0].real() << " (expected " << expected << ")\n";
        cout << "  Level: " << ct_result->GetLevel() << "\n";
        cout << "\n";
    }

    cout << "========================================\n";
    cout << "  DEBUG COMPLETE\n";
    cout << "========================================\n";

    return 0;
}
