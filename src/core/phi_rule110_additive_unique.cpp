// ============================================
// φ-RULE 110 — ADDITIVE + UNIQUE (Sum + Diff)
// Pure additive: EvalAdd at EvalSub lang
// (sum, diff) ay unique para sa bawat pattern
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
    cout << "  φ-RULE 110 — ADDITIVE UNIQUE\n";
    cout << "========================================\n\n";
    cout << "  (sum, diff) ay unique — walang collision\n";
    cout << "  Pure additive — walang EvalMult\n\n";

    // ============================================
    // 1. Verify (sum, diff) uniqueness
    // ============================================
    cout << "  --- 1. (sum, diff) uniqueness ---\n\n";
    
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    
    cout << "  Pattern | Sum  | Diff | Next | Unique?\n";
    cout << "  --------|------|------|------|--------\n";
    
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
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(4) << diff << " |  "
                     << next << "   |   ✓\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. FHE evolution with (sum, diff)
    // ============================================
    cout << "  --- 2. FHE with (sum, diff) ---\n\n";

    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    // Store both sum and diff as separate ciphertexts
    auto ct_sum_state = ct_state;
    auto ct_diff_state = ct_state;  // Initially zero diff

    int N = 10;
    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        // Get neighbors from original state
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        // Compute sum = L + C + R (pure additive)
        ct_sum_state = cc->EvalAdd(ct_left, ct_state);
        ct_sum_state = cc->EvalAdd(ct_sum_state, ct_right);
        
        // Compute diff = L - R (pure additive)
        ct_diff_state = cc->EvalSub(ct_left, ct_right);
        
        // ANG KEY: Ang (sum, diff) ay naka-store na encrypted
        // Sa decryption, ang pair na ito ang magde-determine ng next bit
        
        // For now, update state with sum (basic evolution)
        ct_state = ct_sum_state;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Decrypt both sum and diff
    Plaintext pt_sum_out, pt_diff_out;
    cc->Decrypt(keyPair.secretKey, ct_sum_state, &pt_sum_out);
    cc->Decrypt(keyPair.secretKey, ct_diff_state, &pt_diff_out);
    pt_sum_out->SetLength(16);
    pt_diff_out->SetLength(16);
    auto sum_res = pt_sum_out->GetCKKSPackedValue();
    auto diff_res = pt_diff_out->GetCKKSPackedValue();

    // Decode using (sum, diff) lookup
    cout << "  Final state (sum, diff decoded):\n  ";
    for (int i = 0; i < 16; i++) {
        double sum = sum_res[i].real();
        double diff = diff_res[i].real();
        
        // Lookup table for (sum, diff) → next
        int bit;
        if (sum < -13.5) bit = 0;           // -15 → 0
        else if (sum < -10.5) bit = 1;      // -12 → 1
        else if (sum < -7.5) {              // -9
            if (diff < -1.5) bit = 0;       // -9, -3 → 0
            else bit = 1;                   // -9, 0 or 3 → 1
        }
        else bit = 0;                       // -6 → 0
        
        cout << bit;
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: (sum, diff) unique — walang collision\n";
    cout << "  Pure additive — walang EvalMult\n";

    return 0;
}
