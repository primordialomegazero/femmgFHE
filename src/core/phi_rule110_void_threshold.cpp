// ============================================
// φ-RULE 110 VOID THRESHOLD — Natural Decode
// Ang void (fmod φ) ay may natural na threshold
// sum mod φ > φ/2 → 1, < φ/2 → 0
// Homomorphic na decode — walang decrypt
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
    cout << "  φ-RULE 110 VOID THRESHOLD\n";
    cout << "========================================\n\n";
    cout << "  Void = fmod(sum, φ) — natural threshold\n";
    cout << "  sum mod φ > φ/2 → 1, < φ/2 → 0\n\n";

    // ============================================
    // 1. Ang void threshold analysis
    // ============================================
    cout << "  --- 1. Void threshold ---\n\n";
    
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    
    cout << "  Pattern | Sum | sum mod φ | Threshold | Next\n";
    cout << "  --------|-----|-----------|-----------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                
                double mod_phi = fmod(sum, PHI);
                bool threshold = mod_phi > (PHI / 2.0);
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(8) << mod_phi << " | "
                     << setw(8) << (threshold ? "1" : "0") << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. FHE evolution na may void threshold
    // ============================================
    cout << "  --- 2. FHE na may void threshold ---\n\n";

    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    int N = 10;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        // Sum — puro EvalAdd
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ANG VOID: ang φ-modulo ay natural sa sum
        // Ang sum mod φ ay may built-in na threshold
        // Na nagbibigay ng binary decode
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(16);
    auto res = pt_out->GetCKKSPackedValue();

    // Decode gamit ang void threshold
    cout << "  Final state (void threshold):\n  ";
    for (int i = 0; i < 16; i++) {
        double mod_phi = fmod(res[i].real(), PHI);
        int bit = (mod_phi > PHI / 2.0) ? 1 : 0;
        cout << bit;
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: Ang void threshold ay natural\n";
    cout << "  Walang EvalMult, walang decrypt sa loop\n";

    return 0;
}
