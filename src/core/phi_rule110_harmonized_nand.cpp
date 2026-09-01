// ============================================
// φ-RULE 110 HARMONIZED NAND NETWORK
//
// Rule 110 decomposed into NAND gates,
// tapos φ-harmonized para mag-align.
//
// Key: NAND(a,b) = -(a+b)
// Lahat ng gates galing sa NAND
// φ-harmonization ang bahala sa alignment
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
    cout << "  φ-RULE 110 HARMONIZED NAND\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(55);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  φ-harmonized NAND network\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_log = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // NAND GATES (LOG SPACE)
    // ============================================
    
    auto nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    };
    
    auto not_gate = [&](const Ciphertext<DCRTPoly>& a) {
        return nand(a, a);
    };
    
    auto and_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = nand(a, b);
        return not_gate(nand_ab);
    };
    
    auto or_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = not_gate(a);
        auto not_b = not_gate(b);
        return nand(not_a, not_b);
    };

    // ============================================
    // RULE 110 = φ-HARMONIZED NAND NETWORK
    // ============================================
    //
    // Rule 110 = (¬L ∧ C ∧ ¬R) ∨ (¬L ∧ ¬C ∧ R) ∨ 
    //            (¬L ∧ C ∧ R) ∨ (L ∧ ¬C ∧ ¬R) ∨
    //            (L ∧ ¬C ∧ R) ∨ (L ∧ C ∧ ¬R)
    //
    // Lahat NAND-based, φ-harmonized.
    
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        auto nL = not_gate(L);
        auto nC = not_gate(C);
        auto nR = not_gate(R);
        
        // T1: ¬L ∧ ¬C ∧ R (001)
        auto t1 = and_gate(and_gate(nL, nC), R);
        // T2: ¬L ∧ C ∧ ¬R (010)
        auto t2 = and_gate(and_gate(nL, C), nR);
        // T3: ¬L ∧ C ∧ R (011)
        auto t3 = and_gate(and_gate(nL, C), R);
        // T4: L ∧ ¬C ∧ ¬R (100)
        auto t4 = and_gate(and_gate(L, nC), nR);
        // T5: L ∧ ¬C ∧ R (101)
        auto t5 = and_gate(and_gate(L, nC), R);
        // T6: L ∧ C ∧ ¬R (110)
        auto t6 = and_gate(and_gate(L, C), nR);
        
        // OR lahat: φ-harmonized
        auto or_1 = or_gate(t1, t2);
        auto or_2 = or_gate(or_1, t3);
        auto or_3 = or_gate(or_2, t4);
        auto or_4 = or_gate(or_3, t5);
        return or_gate(or_4, t6);
    };

    // ============================================
    // TRANSITION TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (HARMONIZED NAND)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_log(L);
                auto ct_C = encrypt_log(C);
                auto ct_R = encrypt_log(R);
                
                auto ct_out = rule110_gate(ct_L, ct_C, ct_R);
                double val = decrypt_log(ct_out);
                int output = (val > 0) ? 1 : 0;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
