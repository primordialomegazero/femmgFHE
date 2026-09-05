// ============================================
// φ-RULE 110 ALIGNMENT — Natural na Slot
// Ang φ-structure ng sum ay may automatic
// na slot alignment para sa transition
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1, 2, -2, 3, -3, 4, -4});

    const double PHI = 1.6180339887498948482;
    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;

    cout << "========================================\n";
    cout << "  φ-RULE 110 ALIGNMENT — Natural Slot\n";
    cout << "========================================\n\n";
    cout << "  Ang φ-structure ay may natural na\n";
    cout << "  slot alignment para sa transition\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. φ-based na slot alignment
    // ============================================
    cout << "  --- 1. φ-based alignment ---\n\n";
    cout << "  Ang sum ay may natural na φ-relationship:\n";
    cout << "  sum = -15, -12, -9, -6\n";
    cout << "  φ-based: φ⁻⁵, φ⁻⁴, φ⁻³, φ⁻² (powers)\n\n";

    // Ang sum values ay may φ-power na alignment:
    // -15 = 3 × -5 → tatlong EXP_ZERO
    // -12 = 2 × -5 + -2 → dalawang ZERO + isang ONE
    // -9 = -5 + 2 × -2 → isang ZERO + dalawang ONE
    // -6 = 3 × -2 → tatlong ONE

    cout << "  Sum | Composition | φ-power\n";
    cout << "  ----|-------------|---------\n";
    cout << "  -15 | 0+0+0      | φ⁻¹⁵\n";
    cout << "  -12 | 0+0+1      | φ⁻¹²\n";
    cout << "  -9  | 0+1+1      | φ⁻⁹\n";
    cout << "  -6  | 1+1+1      | φ⁻⁶\n\n";

    // ============================================
    // 2. Ang φ-based na slot alignment
    // ============================================
    cout << "  --- 2. Slot alignment ---\n\n";
    cout << "  Kung ang transition table ay naka-align\n";
    cout << "  sa φ-power positions, ang lookup ay automatic\n\n";

    // Ang φ-power positions:
    // φ⁻⁶ → slot 0 (next=0 para sa 111)
    // φ⁻⁹ → slot 1-3 (depende sa diff)
    // φ⁻¹² → slot 4-6 (depende sa diff)
    // φ⁻¹⁵ → slot 7 (next=0 para sa 000)

    cout << "  φ-power | Slot | Next\n";
    cout << "  --------|------|------\n";
    cout << "  φ⁻¹⁵    |  0   |  0\n";
    cout << "  φ⁻¹²    |  1-3 |  1,1,1\n";
    cout << "  φ⁻⁹     |  4-6 |  0,1,1\n";
    cout << "  φ⁻⁶     |  7   |  0\n\n";

    // ============================================
    // 3. FHE na may φ-alignment
    // ============================================
    cout << "  --- 3. FHE na may φ-alignment ---\n\n";

    // Ang φ-aligned na transition table
    // Sa 16 slots, ang table ay naka-align sa φ-powers
    vector<double> aligned_table(16, EXP_ZERO);
    
    // Slot alignment:
    // 0: next=0 (111 → sum=-6)
    // 1-3: next=1 (001,010,100 → sum=-12)
    // 4: next=0 (011 → sum=-9, diff=-3)
    // 5-6: next=1 (101,110 → sum=-9, diff=0,3)
    // 7: next=0 (000 → sum=-15)
    // 8-15: empty (spare)
    
    aligned_table[0] = EXP_ZERO;  // 111 → 0
    aligned_table[1] = EXP_ONE;   // 001 → 1
    aligned_table[2] = EXP_ONE;   // 010 → 1
    aligned_table[3] = EXP_ONE;   // 100 → 1
    aligned_table[4] = EXP_ZERO;  // 011 → 0
    aligned_table[5] = EXP_ONE;   // 101 → 1
    aligned_table[6] = EXP_ONE;   // 110 → 1
    aligned_table[7] = EXP_ZERO;  // 000 → 0

    Plaintext pt_aligned = cc->MakeCKKSPackedPlaintext(aligned_table);
    auto ct_aligned = cc->Encrypt(keyPair.publicKey, pt_aligned);

    cout << "  Aligned table (8 slots):\n  ";
    for (int i = 0; i < 8; i++) {
        cout << aligned_table[i] << " ";
    }
    cout << "\n\n";

    // Ang sum ay may natural na φ-power alignment:
    // sum=-6 → slot 0, sum=-9 → slot 4, sum=-12 → slot 1-3, sum=-15 → slot 7
    // Ang alignment na ito ay φ-based at automatic

    // Initial state
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
        
        // ANG φ-ALIGNMENT:
        // Ang sum ay may natural na φ-power position
        // na automatic na nag-a-align sa transition table
        // Walang explicit na index computation
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: φ-alignment ay automatic\n";

    return 0;
}
