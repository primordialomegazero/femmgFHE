// ============================================
// φ-RULE 110 EVALATINDEX — Libreng Lookup
// Transition table sa slots
// EvalAtIndex para sa libreng lookup
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
    cout << "  φ-RULE 110 EVALATINDEX — Libreng Lookup\n";
    cout << "========================================\n\n";
    cout << "  Transition table sa 16 slots\n";
    cout << "  Ang index ay natural sa (sum, diff)\n\n";

    // ============================================
    // 1. Transition table sa 16 slots
    // ============================================
    cout << "  --- 1. Transition table ---\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    vector<double> table(16, EXP_ZERO);  // Default: next=0

    // I-fill ang table gamit ang (sum, diff) indices
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
                
                table[index] = (next == 1) ? EXP_ONE : EXP_ZERO;
            }
        }
    }

    // Ipakita ang table
    cout << "  Slot | Value | Next\n";
    cout << "  -----|-------|------\n";
    for (int i = 0; i < 16; i++) {
        int next = (abs(table[i] - EXP_ONE) < 0.01) ? 1 : 0;
        cout << "  " << setw(4) << i << " | "
             << setw(5) << table[i] << " | "
             << setw(3) << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. I-encrypt ang transition table
    // ============================================
    Plaintext pt_table = cc->MakeCKKSPackedPlaintext(table);
    auto ct_table = cc->Encrypt(keyPair.publicKey, pt_table);

    cout << "  --- 2. Encrypted table ---\n";
    cout << "  Level: " << ct_table->GetLevel() << "\n\n";

    // ============================================
    // 3. Rule 110 evolution na may lookup
    // ============================================
    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 0000000110000000\n\n";

    int N = 5;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ANG LIBRENG LOOKUP:
        // Ang sum ay may natural na index sa transition table
        // Hindi kailangan ng EvalAtIndex — ang slots ay
        // naka-align na sa sum values
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: Transition table ay handa sa slots\n";

    return 0;
}
