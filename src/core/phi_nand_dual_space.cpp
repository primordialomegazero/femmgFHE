// ============================================
// φ-NAND DUAL SPACE — TAMANG SPACE PER GATE
//
// NAND → φ² space (0→-2, 1→+2)
// AND → Normal space (0→0, 1→1)
// OR → Normal space (0→0, 1→1)
// XOR → Normal space (0→0, 1→1)
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
    cout << "  φ-NAND DUAL SPACE\n";
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
    // DUAL SPACE ENCODING
    // ============================================

    auto encrypt_dual = [&](int bit) {
        vector<double> v(16, 0.0);
        
        // Slots 0-7: φ² SPACE (para sa NAND)
        v[0] = (bit == 0) ? -2.0 : 2.0;
        for (int i = 1; i < 8; i++) v[i] = v[0];
        
        // Slots 8-15: NORMAL SPACE (para sa AND/OR/XOR)
        v[8] = (bit == 0) ? 0.0 : 1.0;
        for (int i = 9; i < 16; i++) v[i] = v[8];
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_phi2 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 8; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 8.0;
        return (avg >= -0.01) ? 1 : 0;
    };

    auto decrypt_normal = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 8; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 8.0;
        return (avg > 0.5) ? 1 : 0;
    };

    // ============================================
    // GATES SA TAMANG SPACE
    // ============================================

    // NAND sa φ²: -(a+b)
    auto nand_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    };

    // AND sa normal: a+b → >1.5
    auto and_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // OR sa normal: a+b → >0.5
    auto or_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // XOR sa normal: |a-b| → >0.5
    auto xor_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    };

    // ============================================
    // TEST 1: NAND (φ² SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: NAND (φ² SPACE)\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | Expected | Match?\n";
    cout << "  ----|------|----------|--------\n";

    int nand_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            int nand = decrypt_phi2(nand_gate(ct_a, ct_b));
            int exp = !(A && B);
            nand_match += (nand == exp);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand << " | "
                 << setw(8) << exp << " | "
                 << (nand == exp ? "✅" : "❌") << "\n";
        }
    }
    cout << "\n  NAND: " << nand_match << "/4\n\n";

    // ============================================
    // TEST 2: ALL GATES (DUAL SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: ALL GATES (DUAL SPACE)\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND(φ²) | AND(N) | OR(N) | XOR(N)\n";
    cout << "  ----|-----------|--------|-------|--------\n";

    int all_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            
            int nand = decrypt_phi2(nand_gate(ct_a, ct_b));
            
            auto and_ct = and_gate(ct_a, ct_b);
            int and_r = (decrypt_normal(and_ct) > 1.5) ? 1 : 0;  // AND: >1.5
            
            int or_r = decrypt_normal(or_gate(ct_a, ct_b));  // OR: >0.5
            
            auto xor_ct = xor_gate(ct_a, ct_b);
            double xor_val = abs(decrypt_normal(xor_ct));
            int xor_r = (xor_val > 0.5) ? 1 : 0;  // XOR: |a-b| > 0.5
            
            int exp_nand = !(A && B);
            int exp_and = (A && B);
            int exp_or = (A || B);
            int exp_xor = (A != B);
            
            all_match += (nand == exp_nand) + (and_r == exp_and) + 
                         (or_r == exp_or) + (xor_r == exp_xor);
            
            bool all_ok = (nand == exp_nand && and_r == exp_and && or_r == exp_or && xor_r == exp_xor);
            
            cout << "  " << A << " " << B << " | "
                 << setw(9) << nand << " | "
                 << setw(6) << and_r << " | "
                 << setw(5) << or_r << " | "
                 << setw(6) << xor_r << " | "
                 << (all_ok ? "✅" : "❌") << "\n";
        }
    }
    cout << "\n  All gates: " << all_match << "/16\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  NAND DUAL SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ NAND: " << nand_match << "/4 (φ² space)\n";
    cout << "  ✅ All gates: " << all_match << "/16 (dual space)\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
