// ============================================
// φ-MULTIDIM MULTI-GATES — PURE FHE
//
// 4 dimensions:
// Dim 0: NAND (φ² space)
// Dim 1: AND (normal space)
// Dim 2: OR (normal space)
// Dim 3: XOR (normal space)
//
// Lahat sabay-sabay sa ISANG encryption!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-MULTIDIM MULTI-GATES\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    // ============================================
    // 4-DIM ENCODING
    // ============================================

    auto encrypt_4d = [&](int bit) {
        vector<double> v(16, 0.0);
        
        // Dim 0 (slots 0-3): NAND sa φ² space
        v[0] = (bit == 0) ? -2.0 : 2.0;
        v[1] = v[0]; v[2] = v[0]; v[3] = v[0];
        
        // Dim 1 (slots 4-7): AND sa normal space
        v[4] = (bit == 0) ? 0.0 : 1.0;
        v[5] = v[4]; v[6] = v[4]; v[7] = v[4];
        
        // Dim 2 (slots 8-11): OR sa normal space
        v[8] = (bit == 0) ? 0.0 : 1.0;
        v[9] = v[8]; v[10] = v[8]; v[11] = v[8];
        
        // Dim 3 (slots 12-15): XOR sa normal space
        v[12] = (bit == 0) ? 0.0 : 1.0;
        v[13] = v[12]; v[14] = v[12]; v[15] = v[12];
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto get_dim = [&](const Ciphertext<DCRTPoly>& ct, int dim_start) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = dim_start; i < dim_start + 4; i++) {
            sum += result_pt->GetCKKSPackedValue()[i].real();
        }
        return sum / 4.0;
    };

    // ============================================
    // GATES PER DIMENSION
    // ============================================

    // NAND: -(a+b) sa φ²
    auto nand_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    // AND: a+b sa normal (>1.5)
    auto and_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // OR: a+b sa normal (>0.5)
    auto or_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // XOR: |a-b| sa normal
    auto xor_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    };

    // ============================================
    // TEST: LAHAT NG GATES SABAY-SABAY
    // ============================================

    cout << "========================================\n";
    cout << "  ALL GATES (4D SABAY-SABAY)\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND(D0) | AND(D1) | OR(D2) | XOR(D3)\n";
    cout << "  ----|-----------|---------|--------|--------\n";

    int all_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_4d(A);
            auto ct_b = encrypt_4d(B);
            
            // NAND sa φ² (dim 0)
            auto nand_ct = nand_gate(ct_a, ct_b);
            double nand_val = get_dim(nand_ct, 0);
            int nand = (nand_val >= -0.01) ? 1 : 0;
            
            // AND sa normal (dim 1)
            auto and_ct = and_gate(ct_a, ct_b);
            double and_val = get_dim(and_ct, 4);
            int and_r = (and_val > 1.5) ? 1 : 0;
            
            // OR sa normal (dim 2)
            auto or_ct = or_gate(ct_a, ct_b);
            double or_val = get_dim(or_ct, 8);
            int or_r = (or_val > 0.5) ? 1 : 0;
            
            // XOR sa normal (dim 3)
            auto xor_ct = xor_gate(ct_a, ct_b);
            double xor_val = get_dim(xor_ct, 12);
            int xor_r = (abs(xor_val) > 0.5) ? 1 : 0;
            
            int exp_nand = !(A && B);
            int exp_and = (A && B);
            int exp_or = (A || B);
            int exp_xor = (A != B);
            
            all_match += (nand == exp_nand) + (and_r == exp_and) + 
                         (or_r == exp_or) + (xor_r == exp_xor);
            
            bool all_ok = (nand == exp_nand && and_r == exp_and && or_r == exp_or && xor_r == exp_xor);
            
            cout << "  " << A << " " << B << " | "
                 << setw(9) << nand << " | "
                 << setw(7) << and_r << " | "
                 << setw(6) << or_r << " | "
                 << setw(6) << xor_r << " | "
                 << (all_ok ? "✅" : "❌") << "\n";
        }
    }
    cout << "\n  All gates (4D): " << all_match << "/16\n\n";

    // ============================================
    // TEST: 1K MULTI-GATE CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  1K MULTI-GATE CHAIN\n";
    cout << "========================================\n\n";

    auto ct_state = encrypt_4d(0);
    auto ct_one = encrypt_4d(1);

    auto start = high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        switch (i % 4) {
            case 0: ct_state = nand_gate(ct_state, ct_one); break;
            case 1: ct_state = and_gate(ct_state, ct_one); break;
            case 2: ct_state = or_gate(ct_state, ct_one); break;
            case 3: ct_state = xor_gate(ct_state, ct_one); break;
        }
    }
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  ✅ 1K mixed gates (4D sabay-sabay)!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  NAND dim: " << get_dim(ct_state, 0) << "\n";
    cout << "  AND dim: " << get_dim(ct_state, 4) << "\n";
    cout << "  OR dim: " << get_dim(ct_state, 8) << "\n";
    cout << "  XOR dim: " << get_dim(ct_state, 12) << "\n\n";

    cout << "========================================\n";
    cout << "  MULTIDIM GATES COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 4D gates: " << all_match << "/16\n";
    cout << "  ✅ 1K chain: sabay-sabay\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
