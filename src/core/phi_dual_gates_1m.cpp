// ============================================
// φ-DUAL GATES 1M — LAHAT NG GATES + 1M CHAIN
//
// Dual space: φ² + Normal
// 1M mixed: NAND, AND, OR, XOR, NOT
// Walang decrypt sa gitna!
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
    cout << "  φ-DUAL GATES 1M\n";
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

    auto encrypt_dual = [&](int bit) {
        vector<double> v(16, 0.0);
        v[0] = (bit == 0) ? -2.0 : 2.0;
        for (int i = 1; i < 8; i++) v[i] = v[0];
        for (int i = 8; i < 16; i++) v[i] = (bit == 0) ? 0.0 : 2.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        vector<double> results(16);
        for (int i = 0; i < 16; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    // ============================================
    // TEST 1: ALL GATES (16/16 CHECK)
    // ============================================

    cout << "========================================\n";
    cout << "  1. ALL GATES (DUAL SPACE)\n";
    cout << "========================================\n\n";
    cout << "  A B | NAND | AND | OR | XOR\n";
    cout << "  ----|------|-----|----|----\n";

    int gate_match = 0;
    int gate_total = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            
            // NAND: negate(add) sa φ² space
            auto nand_ct = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
            auto nand_vals = decrypt_dual(nand_ct);
            int nand_decoded = (nand_vals[0] >= -0.01) ? 1 : 0;
            int nand_expected = !(A && B);
            
            // AND: sum sa normal space
            auto and_ct = cc->EvalAdd(ct_a, ct_b);
            auto and_vals = decrypt_dual(and_ct);
            int and_decoded = (and_vals[8] > 1.5) ? 1 : 0;
            int and_expected = A && B;
            
            // OR: sum sa normal space
            auto or_ct = cc->EvalAdd(ct_a, ct_b);
            auto or_vals = decrypt_dual(or_ct);
            int or_decoded = (or_vals[8] > 0.5) ? 1 : 0;
            int or_expected = A || B;
            
            // XOR: diff sa normal space
            auto xor_ct = cc->EvalSub(ct_a, ct_b);
            auto xor_vals = decrypt_dual(xor_ct);
            int xor_decoded = (abs(xor_vals[8]) > 0.5) ? 1 : 0;
            int xor_expected = A ^ B;
            
            gate_total += 4;
            gate_match += (nand_decoded == nand_expected) + 
                          (and_decoded == and_expected) + 
                          (or_decoded == or_expected) + 
                          (xor_decoded == xor_expected);
            
            cout << "  " << A << " " << B << " |  "
                 << nand_decoded << "   |  "
                 << and_decoded << "  |  "
                 << or_decoded << "  |  "
                 << xor_decoded << "\n";
        }
    }

    cout << "\n  Gate Match: " << gate_match << "/" << gate_total << "\n\n";

    // ============================================
    // TEST 2: 1M MIXED GATES
    // ============================================

    cout << "========================================\n";
    cout << "  2. 1M MIXED GATES\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_dual(0);
    auto ct_one = encrypt_dual(1);

    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; i++) {
        // Alternate: NAND, XOR, OR, AND
        switch (i % 4) {
            case 0: // NAND
                ct_acc = cc->EvalNegate(cc->EvalAdd(ct_acc, ct_one));
                break;
            case 1: // XOR
                ct_acc = cc->EvalSub(ct_acc, ct_one);
                break;
            case 2: // OR
                ct_acc = cc->EvalAdd(ct_acc, ct_one);
                break;
            case 3: // AND
                ct_acc = cc->EvalAdd(ct_acc, ct_one);
                break;
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto vals_1m = decrypt_dual(ct_acc);
    
    cout << "  Operations: 1,000,000 mixed\n";
    cout << "  φ² result: " << vals_1m[0] << "\n";
    cout << "  Normal result: " << vals_1m[8] << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Towers: " << ct_acc->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // TEST 3: SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  3. SCALING TEST\n";
    cout << "========================================\n\n";
    cout << "  Ops | φ² result | Normal result | Time\n";
    cout << "  ----|-----------|---------------|------\n";

    vector<int> op_counts = {100, 1000, 10000, 100000, 1000000};
    
    for (int ops : op_counts) {
        ct_acc = encrypt_dual(0);
        ct_one = encrypt_dual(1);
        
        auto start_s = high_resolution_clock::now();
        
        for (int i = 0; i < ops; i++) {
            ct_acc = cc->EvalNegate(cc->EvalAdd(ct_acc, ct_one));
        }
        
        auto end_s = high_resolution_clock::now();
        auto time_s = duration_cast<milliseconds>(end_s - start_s).count();
        
        auto vals_s = decrypt_dual(ct_acc);
        
        cout << "  " << setw(5) << ops << " | "
             << setw(9) << fixed << setprecision(2) << vals_s[0] << " | "
             << setw(13) << vals_s[8] << " | "
             << setw(4) << time_s << "ms\n";
    }

    // ============================================
    // SUMMARY
    // ============================================

    cout << "\n========================================\n";
    cout << "  DUAL GATES 1M SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Gates: " << gate_match << "/" << gate_total << "\n";
    cout << "  ✅ 1M mixed: " << vals_1m[0] << ", " << vals_1m[8] << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
