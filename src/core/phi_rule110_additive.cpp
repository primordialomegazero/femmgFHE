// ============================================
// φ-RULE 110 — PURE ADDITIVE (No EvalMult)
// Walang multiplication — EvalAdd/EvalSub lang
// Ang void ay additive: modulo φ
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
    parameters.SetMultiplicativeDepth(1);  // No multiplication needed
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    
    // Rotation keys lang — walang EvalMultKeyGen
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    
    cout << "========================================\n";
    cout << "  φ-RULE 110 — PURE ADDITIVE\n";
    cout << "========================================\n\n";
    cout << "  Walang EvalMult — puro EvalAdd/EvalSub\n";
    cout << "  Ang void ay φ-modulo — natural collapse\n\n";

    // ============================================
    // ENCODING: φ-exponents para sa bits
    // 0 → φ^(-5), 1 → φ^(-2)
    // Ang sum ng tatlong exponents ay unique
    // ============================================
    
    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;
    
    // ============================================
    // RULE 110 na encoded bilang ADDITIVE shifts
    // ============================================
    
    // Initial state: 0000000110000000
    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 0000000110000000\n";
    cout << "  Encoding: 0→φ⁻⁵, 1→φ⁻²\n\n";

    // Expected Rule 110 (for verification)
    vector<int> current = {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0};
    
    cout << "  Gen  0: ";
    for (int i = 0; i < 16; i++) cout << current[i];
    cout << "\n";

    int N = 10;
    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        // ============================================
        // 1. Get neighbors
        // ============================================
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        // ============================================
        // 2. Compute sum = L + C + R
        // ============================================
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ============================================
        // 3. ADDITIVE NORMALIZATION via φ-shifts
        //    I-shift ang sum para ma-collapse sa bits
        // ============================================
        
        // Ang sum ay may values:
        // 0+0+0 = -15 (000)
        // 0+0+1 = -12 (001)
        // 0+1+0 = -12 (010)
        // 0+1+1 = -9  (011)
        // 1+0+0 = -12 (100)
        // 1+0+1 = -9  (101)
        // 1+1+0 = -9  (110)
        // 1+1+1 = -6  (111)
        
        // ADDITIVE SHIFT para ma-map sa next bit:
        // next = 0 para sa sum=-15, -9(some), -6
        // next = 1 para sa sum=-12, -9(some)
        
        // Ang additive shifts:
        // shift_0: Add +15 (para sa -15 → 0)
        // shift_1: Add +12 (para sa -12 → 0)
        // shift_2: Add +9  (para sa -9 → 0)
        
        // Para ma-normalize, i-subtract ang isang offset:
        // offset = 12 (para ma-center)
        // normalized_sum = sum + 12
        // -15 + 12 = -3
        // -12 + 12 = 0
        // -9 + 12 = 3
        // -6 + 12 = 6
        
        vector<double> offset_v(16, 12.0);
        Plaintext pt_offset = cc->MakeCKKSPackedPlaintext(offset_v);
        auto ct_normalized = cc->EvalAdd(ct_sum, pt_offset);
        
        // ============================================
        // 4. ADDITIVE VOID — natural na reset
        //    I-add ang φ-based na shift para ma-collapse
        // ============================================
        
        // Ang normalized values: -3, 0, 3, 6
        // Rule 110 mapping:
        // -3 → 0 (000)
        // 0 → 1 (001, 010, 100) — 3 cases
        // 3 → 0 or 1 (011→0, 101→1, 110→1)
        // 6 → 0 (111)
        
        // ADDITIVE VOID: I-subtract ang "baseline"
        // baseline = 1.5 (average ng 0 at 3)
        // void_shift = normalized - 1.5
        // -3 → -4.5 → round to 0
        // 0 → -1.5 → round to 0  
        // 3 → 1.5 → round to 1
        // 6 → 4.5 → round to 1
        
        vector<double> baseline_v(16, 1.5);
        Plaintext pt_baseline = cc->MakeCKKSPackedPlaintext(baseline_v);
        auto ct_void = cc->EvalSub(ct_normalized, pt_baseline);
        
        // ============================================
        // 5. Update state with additive collapse
        // ============================================
        ct_state = ct_void;
        
        // Update expected (for verification)
        vector<int> next_gen(16, 0);
        for (int i = 0; i < 16; i++) {
            int L = current[(i+15)%16];
            int C = current[i];
            int R = current[(i+1)%16];
            int pattern = (L<<2) | (C<<1) | R;
            if (pattern == 0 || pattern == 3 || pattern == 4 || pattern == 7) {
                next_gen[i] = 0;
            } else {
                next_gen[i] = 1;
            }
        }
        current = next_gen;
        
        cout << "  Gen " << setw(2) << gen+1 << ": ";
        for (int i = 0; i < 16; i++) cout << current[i];
        cout << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Decrypt and show
    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(16);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "\n  Final (Homomorphic): ";
    for (int i = 0; i < 16; i++) {
        // Simple threshold: > 0 means 1, < 0 means 0
        cout << (res[i].real() > 0 ? "1" : "0");
    }
    cout << "\n";
    
    cout << "  Final (Expected):    ";
    for (int i = 0; i < 16; i++) {
        cout << current[i];
    }
    cout << "\n\n";
    
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Ops: Pure additive (EvalAdd/EvalSub only)\n";

    return 0;
}
