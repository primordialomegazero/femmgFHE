// ============================================
// φ-ZERO-LEVEL DIVISION VIA NEWTON
// y_{n+1} = y_n × (2 - x × y_n)
// Lahat ng multiplication ay zero-level
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
    cout << "  φ-ZERO-LEVEL DIVISION VIA NEWTON\n";
    cout << "  y_{n+1} = y_n × (2 - x × y_n)\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
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

    // Zero-level scalar multiply via doubling
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

    // Zero-level multiplication: ct_a × scalar
    auto zero_mult = [&](Ciphertext<DCRTPoly> ct, long long scalar) {
        vector<int> terms;
        greedy_class1(scalar, terms);

        Ciphertext<DCRTPoly> result;
        bool first = true;

        for (int idx : terms) {
            Ciphertext<DCRTPoly> partial = scalar_multiply(ct, L[idx]);
            if (first) {
                result = partial;
                first = false;
            } else {
                result = cc->EvalAdd(result, partial);
            }
        }

        return result;
    };

    cout << "========================================\n";
    cout << "  TEST: COMPUTE 1/7\n";
    cout << "========================================\n\n";

    double target = 7.0;
    double expected_recip = 1.0 / target;

    // Encrypt initial guess y_0 = 0.1
    vector<double> plain_y(slots, 0.1);
    auto ct_y = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_y));

    // Encrypt x = 7
    vector<double> plain_x(slots, 7.0);
    auto ct_x = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_x));

    // Encrypt 2
    vector<double> plain_two(slots, 2.0);
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    cout << "Initial guess: 0.1\n";
    cout << "Target: 1/7 = " << fixed << setprecision(6) << expected_recip << "\n\n";

    cout << "NEWTON ITERATIONS (zero-level):\n";
    cout << "Step | Value | Expected | Error | Level\n";
    cout << "-----|-------|----------|-------|-------\n";

    // Newton iteration with MIXED approach:
    // Traditional EvalMult for y(2-xy) kasi kailangan ng ct×ct
    // Pero kung ang x ay scalar, zero-level ang multiply

    for (int i = 0; i <= 5; i++) {
        // Decrypt to check
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_y, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();
        double error = abs(val - expected_recip);

        cout << setw(4) << i << " | "
             << setw(10) << fixed << setprecision(6) << val << " | "
             << setw(10) << fixed << setprecision(6) << expected_recip << " | "
             << setw(10) << scientific << setprecision(2) << error << " | "
             << setw(5) << ct_y->GetLevel() << "\n";

        if (i < 5) {
            // y = y(2 - 7y)
            // Sa zero-level approach: 7y ay scalar multiply
            auto ct_7y = zero_mult(ct_y, 7);  // Zero-level
            auto ct_factor = cc->EvalSub(ct_two, ct_7y);  // 2 - 7y

            // y × factor
            // Kung factor ay integer, zero-level
            // Pero factor ay encrypted at may decimals
            // Kailangan ng traditional multiply dito
            ct_y = cc->EvalMult(ct_y, ct_factor);
        }
    }

    cout << "\n========================================\n";
    cout << "  ANALYSIS\n";
    cout << "========================================\n\n";
    cout << "  Zero-level parts:\n";
    cout << "  - 7y via scalar multiply: zero-level ✅\n";
    cout << "  - 2 - 7y via subtraction: zero-level ✅\n\n";
    cout << "  Level-consuming parts:\n";
    cout << "  - y × (2-7y) na encrypted × encrypted: 1 level ❌\n\n";
    cout << "  Ang huling multiply ay kailangan ng\n";
    cout << "  traditional EvalMult kasi pareho silang\n";
    cout << "  encrypted na may decimal values.\n";
    cout << "========================================\n";

    return 0;
}
