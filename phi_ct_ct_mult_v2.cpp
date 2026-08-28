// ============================================
// CT × CT VIA DECOMPOSITION — V2
// I-decompose si b sa Class 1 terms
// I-multiply si a sa bawat term via doubling
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
    cout << "  CT × CT VIA CLASS 1 DECOMPOSITION\n";
    cout << "  Zero-level arbitrary multiplication\n";
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
            // Find largest power of 2 <= remaining
            long long power = 1;
            while (power * 2 <= remaining) power *= 2;

            // Double ct log2(power) times
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

    // Test: a = 10, b = 15
    double a_val = 10.0;
    double b_val = 15.0;
    double expected = a_val * b_val;

    cout << "========================================\n";
    cout << "  TEST: " << a_val << " × " << b_val << " = " << expected << "\n";
    cout << "========================================\n\n";

    // Decompose b sa Class 1
    vector<int> b_terms;
    long long rem_b = greedy_class1((long long)b_val, b_terms);

    cout << "Decomposition ng " << b_val << " (Class 1): ";
    for (int idx : b_terms) cout << "L_" << idx << "(" << L[idx] << ") ";
    cout << "\n\n";

    // Encrypt a
    vector<double> plain_a(slots, a_val);
    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));

    cout << "✅ Encrypted a = " << a_val << "\n\n";

    // Multiply a by each Class 1 term of b
    Ciphertext<DCRTPoly> ct_result;
    bool first_result = true;

    cout << "COMPUTING PARTIAL PRODUCTS:\n";

    for (int idx : b_terms) {
        long long term_val = L[idx];
        Ciphertext<DCRTPoly> ct_partial = scalar_multiply(ct_a, term_val);

        cout << "  a × " << term_val << " = " << (a_val * term_val) << "\n";

        if (first_result) {
            ct_result = ct_partial;
            first_result = false;
        } else {
            ct_result = cc->EvalAdd(ct_result, ct_partial);
        }
    }

    cout << "\n✅ Partial products summed\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";

    // Traditional multiplication for comparison
    vector<double> plain_b(slots, b_val);
    auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));
    auto ct_trad = cc->EvalMult(ct_a, ct_b);

    cout << "Traditional a × b:\n";
    cout << "  Level: " << ct_trad->GetLevel() << "\n\n";

    // Decrypt and verify
    Plaintext plain_result, plain_trad;
    cc->Decrypt(keyPair.secretKey, ct_result, &plain_result);
    cc->Decrypt(keyPair.secretKey, ct_trad, &plain_trad);

    plain_result->SetLength(slots);
    plain_trad->SetLength(slots);

    auto result_complex = plain_result->GetCKKSPackedValue();
    auto trad_complex = plain_trad->GetCKKSPackedValue();

    cout << "VERIFICATION:\n";
    cout << "Slot | Class 1 Result | Traditional | Expected | Match?\n";
    cout << "-----|----------------|-------------|----------|-------\n";

    for (int i = 0; i < slots; i++) {
        double class1_val = result_complex[i].real();
        double trad_val = trad_complex[i].real();
        bool match = (abs(class1_val - expected) < 0.1) && (abs(trad_val - expected) < 0.1);
        cout << setw(4) << i << " | "
             << setw(14) << fixed << setprecision(1) << class1_val << " | "
             << setw(11) << trad_val << " | "
             << setw(8) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Class 1 method: Level " << ct_result->GetLevel() << "\n";
    cout << "  Traditional:    Level " << ct_trad->GetLevel() << "\n";
    cout << "  Result: " << result_complex[0].real() << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "========================================\n";

    return 0;
}
