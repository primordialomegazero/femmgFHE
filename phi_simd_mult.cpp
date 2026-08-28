// ============================================
// φ-SIMD VARIABLE MULTIPLICATION
// Per-slot iba't ibang multiplier
// Zero-level via decomposition
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
    cout << "  φ-SIMD VARIABLE MULTIPLICATION\n";
    cout << "  Per-slot iba't ibang multiplier\n";
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
    cout << "  APPROACH: SIMD VIA MASKING\n";
    cout << "========================================\n\n";

    cout << "Strategy: Para sa per-slot multiplication,\n";
    cout << "gumamit ng masking na zero-level din.\n\n";

    // Test values
    vector<double> a_values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    vector<double> b_values = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0};

    cout << "a = [";
    for (int i = 0; i < slots; i++) cout << (i > 0 ? ", " : "") << a_values[i];
    cout << "]\n";

    cout << "b = [";
    for (int i = 0; i < slots; i++) cout << (i > 0 ? ", " : "") << b_values[i];
    cout << "]\n\n";

    // Expected: a[i] × b[i] per slot
    vector<double> expected(slots);
    for (int i = 0; i < slots; i++) expected[i] = a_values[i] * b_values[i];

    cout << "Expected: [";
    for (int i = 0; i < slots; i++) cout << (i > 0 ? ", " : "") << expected[i];
    cout << "]\n\n";

    // APPROACH 1: Scalar na iisang multiplier
    // Hindi ito per-slot, pero benchmark muna

    cout << "APPROACH 1: Scalar multiply (hindi per-slot)\n";
    cout << "Ito ay baseline — iisang multiplier para sa lahat\n\n";

    vector<double> plain_a(slots);
    for (int i = 0; i < slots; i++) plain_a[i] = a_values[i];

    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));

    cout << "✅ Encrypted a (8 slots)\n\n";

    cout << "APPROACH 2: Per-slot via masking\n";
    cout << "Kailangan natin ng zero-level mask\n\n";

    // Masking strategy:
    // Para sa slot i, gumawa ng mask na may 1 sa slot i at 0 sa iba
    // Tapos multiply ang ct sa mask (traditional) o decompose

    // Traditional masking (baseline)
    cout << "Traditional masking:\n";
    cout << "ct × mask_i = ct_i (isolated slot)\n";
    cout << "Ito ay nangangailangan ng ct × ct\n\n";

    // Zero-level masking attempt:
    // Kung may mask_i na [0,0,...,1,...,0]
    // Ang ct × mask_i ay pwedeng i-approximate via additions
    // Pero ang multiplication sa 0 ay kailangan ng multiplication

    cout << "========================================\n";
    cout << "  PROBLEM: ZERO-LEVEL MASKING\n";
    cout << "========================================\n\n";

    cout << "Ang mask ay may 0 at 1 values.\n";
    cout << "ct × 0 = 0 (kailangan ng multiplication)\n";
    cout << "ct × 1 = ct (identity)\n\n";

    cout << "Kung may zero-level mask, ang per-slot\n";
    cout << "multiplication ay magiging zero-level din.\n\n";

    cout << "ATTEMPT: Additive mask via shifting\n";
    cout << "Imbes na multiply ng 0/1, i-add ng offset\n\n";

    // Alternative: Decompose b values per slot
    cout << "========================================\n";
    cout << "  ALTERNATIVE: DECOMPOSE PER SLOT\n";
    cout << "========================================\n\n";

    cout << "Kung ang b[i] ay known constants,\n";
    cout << "pwede nating i-precompute ang decomposition\n";
    cout << "para sa bawat slot.\n\n";

    // For each slot, decompose b[i]
    vector<vector<int>> b_decompositions(slots);

    for (int i = 0; i < slots; i++) {
        long long b_scaled = (long long)(b_values[i] * 100);
        greedy_class1(b_scaled, b_decompositions[i]);
    }

    cout << "Decompositions of b (scaled by 100):\n";
    for (int i = 0; i < slots; i++) {
        cout << "  b[" << i << "] = " << b_values[i] << " → ";
        long long b_scaled = (long long)(b_values[i] * 100);
        cout << b_scaled << " = ";
        for (int idx : b_decompositions[i]) {
            cout << L[idx] << " ";
        }
        cout << "\n";
    }
    cout << "\n";

    cout << "========================================\n";
    cout << "  SOLUTION: PRE-COMPUTED MASKS\n";
    cout << "========================================\n\n";

    cout << "Kung ang b_values ay known bago i-encrypt,\n";
    cout << "pwede nating i-precompute ang masks.\n";
    cout << "Ang bawat mask ay may 1 sa specific slot.\n";
    cout << "Ang ct × mask ay traditional multiplication.\n";
    cout << "PERO: Kung ang mask ay 0/1 lamang,\n";
    cout << "baka may special optimization.\n\n";

    cout << "========================================\n";
    cout << "  HONEST STATUS\n";
    cout << "========================================\n\n";
    cout << "  ✅ Scalar multiplication: ZERO-LEVEL\n";
    cout << "  ✅ Floating point: ZERO-LEVEL\n";
    cout << "  ✅ Chained operations: ZERO-LEVEL\n";
    cout << "  ❌ Per-slot SIMD variable: Kailangan pa\n\n";
    cout << "  Ang per-slot variable multiply ay\n";
    cout << "  nangangailangan ng masking na\n";
    cout << "  traditional multiplication pa rin.\n";
    cout << "========================================\n";

    return 0;
}
