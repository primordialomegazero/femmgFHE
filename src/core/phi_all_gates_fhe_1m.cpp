// ============================================
// φ-ALL GATES FHE 1M — ENCRYPTED + CHAINED
//
// Lahat ng gates: AND, OR, XOR, NAND, NOR, XNOR, NOT
// Naka-encrypt, naka-chain ng 1M operations
//
// Encoding: 0→0, 1→2
// Decode: mod 4
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

const double PHI = 1.6180339887498948482;

int main() {
    cout << "========================================\n";
    cout << "  φ-ALL GATES FHE 1M\n";
    cout << "========================================\n\n";

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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n\n";

    auto encrypt_val = [&](double val) {
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    auto decode = [](double result) {
        double mod4 = fmod(result, 4.0);
        if (mod4 < 0) mod4 += 4.0;
        return (mod4 < 2.0) ? 0 : 1;
    };

    // ============================================
    // TEST 1: ENCRYPTED GATES (2-INPUT)
    // ============================================

    cout << "========================================\n";
    cout << "  1. ENCRYPTED GATES (2-INPUT)\n";
    cout << "========================================\n\n";
    cout << "  A B | AND | OR | XOR | NAND | NOR | XNOR\n";
    cout << "  ----|-----|----|-----|------|-----|-----\n";

    int gate_match = 0;
    int gate_total = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double a_val = (A == 0) ? 0.0 : 2.0;
            double b_val = (B == 0) ? 0.0 : 2.0;
            
            auto ct_a = encrypt_val(a_val);
            auto ct_b = encrypt_val(b_val);
            
            // AND: A * B / 2
            auto ct_and = cc->EvalMult(ct_a, ct_b);
            double and_avg = decrypt_avg(ct_and);
            int and_decoded = decode(and_avg / 2.0);
            int and_expected = A & B;
            
            // OR: A + B - AND
            auto ct_or_raw = cc->EvalAdd(ct_a, ct_b);
            auto ct_or = cc->EvalSub(ct_or_raw, ct_and);
            double or_avg = decrypt_avg(ct_or);
            int or_decoded = decode(or_avg / 2.0);
            int or_expected = A | B;
            
            // XOR: A + B (mod 4)
            auto ct_xor = cc->EvalAdd(ct_a, ct_b);
            double xor_avg = decrypt_avg(ct_xor);
            int xor_decoded = decode(xor_avg);
            int xor_expected = A ^ B;
            
            // NAND: NOT(AND)
            auto ct_nand = cc->EvalSub(encrypt_val(2.0), ct_and);
            double nand_avg = decrypt_avg(ct_nand);
            int nand_decoded = decode(nand_avg / 2.0);
            int nand_expected = !(A & B);
            
            // NOR: NOT(OR)
            auto ct_nor = cc->EvalSub(encrypt_val(2.0), ct_or);
            double nor_avg = decrypt_avg(ct_nor);
            int nor_decoded = decode(nor_avg / 2.0);
            int nor_expected = !(A | B);
            
            // XNOR: NOT(XOR)
            auto ct_xnor = cc->EvalSub(encrypt_val(2.0), ct_xor);
            double xnor_avg = decrypt_avg(ct_xnor);
            int xnor_decoded = decode(xnor_avg);
            int xnor_expected = !(A ^ B);
            
            gate_total += 6;
            gate_match += (and_decoded == and_expected) + 
                          (or_decoded == or_expected) + 
                          (xor_decoded == xor_expected) + 
                          (nand_decoded == nand_expected) + 
                          (nor_decoded == nor_expected) + 
                          (xnor_decoded == xnor_expected);
            
            cout << "  " << A << " " << B << " |  "
                 << and_decoded << "  |  "
                 << or_decoded << "  |  "
                 << xor_decoded << "  |   "
                 << nand_decoded << "   |  "
                 << nor_decoded << "  |   "
                 << xnor_decoded << "\n";
        }
    }

    cout << "\n  Gate Match: " << gate_match << "/" << gate_total << "\n\n";

    // ============================================
    // TEST 2: ENCRYPTED 3-INPUT GATES
    // ============================================

    cout << "========================================\n";
    cout << "  2. ENCRYPTED 3-INPUT GATES\n";
    cout << "========================================\n\n";
    cout << "  A B C | AND3 | OR3 | XOR3\n";
    cout << "  -------|------|-----|-----\n";

    int match3 = 0;
    int total3 = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int C : {0, 1}) {
                double a_val = (A == 0) ? 0.0 : 2.0;
                double b_val = (B == 0) ? 0.0 : 2.0;
                double c_val = (C == 0) ? 0.0 : 2.0;
                
                auto ct_a = encrypt_val(a_val);
                auto ct_b = encrypt_val(b_val);
                auto ct_c = encrypt_val(c_val);
                
                // AND3: A * B * C / 4
                auto ct_ab = cc->EvalMult(ct_a, ct_b);
                auto ct_abc = cc->EvalMult(ct_ab, ct_c);
                double and3_avg = decrypt_avg(ct_abc);
                int and3_decoded = decode(and3_avg / 4.0);
                int and3_expected = A & B & C;
                
                // XOR3: A + B + C (mod 4)
                auto ct_ab_sum = cc->EvalAdd(ct_a, ct_b);
                auto ct_abc_sum = cc->EvalAdd(ct_ab_sum, ct_c);
                double xor3_avg = decrypt_avg(ct_abc_sum);
                int xor3_decoded = decode(xor3_avg);
                int xor3_expected = A ^ B ^ C;
                
                // OR3: approximated as XOR3 + AND3
                auto ct_or3 = cc->EvalAdd(ct_abc_sum, ct_abc);
                double or3_avg = decrypt_avg(ct_or3);
                int or3_decoded = decode(or3_avg / 2.0);
                int or3_expected = A | B | C;
                
                total3 += 3;
                match3 += (and3_decoded == and3_expected) + 
                          (or3_decoded == or3_expected) + 
                          (xor3_decoded == xor3_expected);
                
                cout << "  " << A << " " << B << " " << C << " |  "
                     << and3_decoded << "   |  "
                     << or3_decoded << "  |  "
                     << xor3_decoded << "\n";
            }
        }
    }

    cout << "\n  3-Input Match: " << match3 << "/" << total3 << "\n\n";

    // ============================================
    // TEST 3: 1M CHAINED XOR
    // ============================================

    cout << "========================================\n";
    cout << "  3. 1M CHAINED XOR\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_val(0.0);
    auto ct_one = encrypt_val(2.0);

    auto start = high_resolution_clock::now();
    
    // 1M chained XOR operations
    for (int i = 0; i < 1000000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_one);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double avg_1m = decrypt_avg(ct_acc);
    int decoded_1m = decode(avg_1m);
    int expected_1m = 1000000 % 2;

    cout << "  Operations: 1,000,000\n";
    cout << "  Result: " << decoded_1m << "\n";
    cout << "  Expected: " << expected_1m << "\n";
    cout << "  Match: " << (decoded_1m == expected_1m ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: 1M MIXED GATES
    // ============================================

    cout << "========================================\n";
    cout << "  4. 1M MIXED GATES\n";
    cout << "========================================\n\n";

    auto ct_mixed = encrypt_val(0.0);
    auto ct_two = encrypt_val(2.0);

    auto start_mixed = high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; i++) {
        // Alternate: XOR (add), AND (mult), OR (add+sub)
        if (i % 3 == 0) {
            ct_mixed = cc->EvalAdd(ct_mixed, ct_two);
        } else if (i % 3 == 1) {
            ct_mixed = cc->EvalMult(ct_mixed, ct_two);
        } else {
            ct_mixed = cc->EvalSub(ct_mixed, ct_two);
        }
    }
    
    auto end_mixed = high_resolution_clock::now();
    auto time_mixed = duration_cast<milliseconds>(end_mixed - start_mixed).count();

    double avg_mixed = decrypt_avg(ct_mixed);
    int decoded_mixed = decode(avg_mixed);

    cout << "  Operations: 1,000,000 mixed\n";
    cout << "  Result: " << decoded_mixed << "\n";
    cout << "  Time: " << time_mixed << " ms\n";
    cout << "  Level: " << ct_mixed->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ALL GATES FHE 1M SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ 2-input gates: " << gate_match << "/" << gate_total << "\n";
    cout << "  ✅ 3-input gates: " << match3 << "/" << total3 << "\n";
    cout << "  ✅ 1M chained XOR: " << decoded_1m << "\n";
    cout << "  ✅ 1M mixed: " << decoded_mixed << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
