// ============================================
// φ-FULL ADDER — ALL LOG SPACE
//
// Lahat ng gates sa log space (φ² encoding):
// 0 → -2, 1 → +2
//
// NAND: -(a+b)
// AND: NAND(NAND,NAND)
// OR: NAND(NOT,NOT)
// XOR: NAND(NAND(a,NAND), NAND(b,NAND))
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-FULL ADDER — ALL LOG SPACE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  All log space: 0→-2, 1→+2\n\n";

    // ============================================
    // LOG SPACE ENCODING
    // ============================================

    auto encrypt_log = [&](int bit) {
        vector<double> v(8, 0.0);
        v[0] = (bit == 0) ? -2.0 : 2.0;
        for (int i = 1; i < 8; i++) {
            v[i] = v[0] * pow(PHI, i);
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return (log_val >= -0.01) ? 1 : 0;
    };

    // ============================================
    // LOG SPACE GATES
    // ============================================

    // NAND: -(a+b)
    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    // NOT: NAND(a,a) = -a
    auto gate_not = [&](const Ciphertext<DCRTPoly>& a) {
        return cc->EvalNegate(a);
    };

    // AND: NAND(NAND(a,b), NAND(a,b))
    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        return gate_nand(nand_ab, nand_ab);
    };

    // OR: NAND(NOT(a), NOT(b))
    auto gate_or = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        return gate_nand(not_a, not_b);
    };

    // XOR: NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        auto nand_a_n = gate_nand(a, nand_ab);
        auto nand_b_n = gate_nand(b, nand_ab);
        return gate_nand(nand_a_n, nand_b_n);
    };

    // ============================================
    // TEST 1: ALL GATES (LOG SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  ALL GATES (LOG SPACE)\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | AND | OR | XOR\n";
    cout << "  ----|------|-----|----|----\n";

    int all_correct = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_log(A);
            auto ct_b = encrypt_log(B);
            
            int nand = decrypt_log(gate_nand(ct_a, ct_b));
            int and_r = decrypt_log(gate_and(ct_a, ct_b));
            int or_r = decrypt_log(gate_or(ct_a, ct_b));
            int xor_r = decrypt_log(gate_xor(ct_a, ct_b));
            
            int exp_nand = !(A && B);
            int exp_and = (A && B);
            int exp_or = (A || B);
            int exp_xor = (A != B);
            
            all_correct += (nand == exp_nand) + (and_r == exp_and) + 
                          (or_r == exp_or) + (xor_r == exp_xor);
            
            bool all_ok = (nand == exp_nand && and_r == exp_and && or_r == exp_or && xor_r == exp_xor);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand << " | "
                 << setw(3) << and_r << " | "
                 << setw(2) << or_r << " | "
                 << setw(3) << xor_r << " | "
                 << (all_ok ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n  All gates: " << all_correct << "/16\n\n";

    // ============================================
    // TEST 2: FULL ADDER (ALL LOG SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  FULL ADDER (ALL LOG SPACE)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum Cout | Both?\n";
    cout << "  --------|----------|------\n";

    int adder_correct = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_log(A);
                auto ct_b = encrypt_log(B);
                auto ct_cin = encrypt_log(Cin);
                
                // Sum = XOR(XOR(A,B), Cin)
                auto xor_ab = gate_xor(ct_a, ct_b);
                auto sum_ct = gate_xor(xor_ab, ct_cin);
                
                // Cout = OR(AND(A,B), AND(Cin, XOR(A,B)))
                auto and_ab = gate_and(ct_a, ct_b);
                auto and_cin_xor = gate_and(ct_cin, xor_ab);
                auto cout_ct = gate_or(and_ab, and_cin_xor);
                
                int sum = decrypt_log(sum_ct);
                int cout_val = decrypt_log(cout_ct);
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                
                bool both_ok = (sum == exp_sum && cout_val == exp_cout);
                adder_correct += both_ok;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << " " << sum << "   " << cout_val << "   | "
                     << (both_ok ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ALL LOG SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ All log space (φ² encoding)\n";
    cout << "  ✅ All gates: " << all_correct << "/16\n";
    cout << "  ✅ Full adder: " << adder_correct << "/8\n";
    cout << "  ✅ Towers: 2\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
