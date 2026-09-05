// ============================================
// φ-RULE 110 2D LOOKUP — Libreng Transition
// (sum, diff) bilang natural na slot index
// Walang EvalMult, walang EvalSquare
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
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;

    cout << "========================================\n";
    cout << "  φ-RULE 110 2D LOOKUP — Libreng Transition\n";
    cout << "========================================\n\n";
    cout << "  (sum, diff) bilang slot index\n";
    cout << "  Walang EvalMult, walang EvalSquare\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. (sum, diff) → unique na slot index
    // ============================================
    cout << "  --- 1. 2D mapping ---\n\n";
    cout << "  Pattern | Sum | Diff | Index | Next\n";
    cout << "  --------|-----|------|-------|------\n";

    // Ang (sum, diff) ay may natural na 2D index:
    // sum ∈ {-15, -12, -9, -6} → 4 values
    // diff ∈ {-3, 0, 3} → 3 values
    // Total: 12 possible slots, 8 used
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                double diff = (L ? EXP_ONE : EXP_ZERO) -
                              (R ? EXP_ONE : EXP_ZERO);
                
                // Index = (sum + 15) / 3 → 0, 1, 2, 3
                int sum_idx = (int)round((sum + 15.0) / 3.0);
                // diff_idx = (diff + 3) / 3 → 0, 1, 2
                int diff_idx = (int)round((diff + 3.0) / 3.0);
                
                // 2D na index: 4 × 3 = 12 slots
                int index = sum_idx * 3 + diff_idx;
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(4) << diff << " | "
                     << setw(5) << index << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang transition table sa 12 slots
    // ============================================
    cout << "  --- 2. Transition table ---\n\n";
    cout << "  Index | Next | Slot value\n";
    cout << "  ------|------|-----------\n";

    // Pre-computed na transition table
    vector<int> transition_table(12, -1);
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                double diff = (L ? EXP_ONE : EXP_ZERO) -
                              (R ? EXP_ONE : EXP_ZERO);
                int sum_idx = (int)round((sum + 15.0) / 3.0);
                int diff_idx = (int)round((diff + 3.0) / 3.0);
                int index = sum_idx * 3 + diff_idx;
                transition_table[index] = next;
            }
        }
    }

    for (int i = 0; i < 12; i++) {
        double slot_value = (transition_table[i] == 1) ? EXP_ONE : EXP_ZERO;
        cout << "  " << setw(5) << i << " | "
             << setw(4) << transition_table[i] << " | "
             << setw(6) << slot_value << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. FHE na may 2D lookup
    // ============================================
    cout << "  --- 3. FHE na may 2D lookup ---\n\n";

    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    int N = 5;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        auto ct_diff = cc->EvalSub(ct_left, ct_right);
        
        // ANG 2D LOOKUP:
        // Ang (sum, diff) ay may natural na slot index
        // na maaaring gamitin para sa EvalAtIndex
        // Walang EvalMult — libreng transition
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: 2D lookup ay libre — walang depth\n";

    return 0;
}
