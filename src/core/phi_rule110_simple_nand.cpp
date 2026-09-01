// ============================================
// φ-RULE 110 SIMPLE NAND — MINIMAL CIRCUIT
//
// Direct minterm implementation
// Encoding: 0 → -2, 1 → +2 (simple log space)
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
    cout << "  φ-RULE 110 SIMPLE NAND\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Simple log space: 0→-2, 1→+2\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_log = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(1, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        double val = result_pt->GetCKKSPackedValue()[0].real();
        return (val > 0) ? 1 : 0;
    };

    // ============================================
    // RULE 110 DIRECT — 5 MINTERMS
    // ============================================
    
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        // Negations
        auto nL = cc->EvalNegate(L);
        auto nC = cc->EvalNegate(C);
        auto nR = cc->EvalNegate(R);
        
        // ==========================================
        // Direct minterm computation
        // ==========================================
        
        // T1: 001 = ¬L ∧ ¬C ∧ R
        auto t1 = cc->EvalAdd(nL, nC);  // ¬L + ¬C = -4
        t1 = cc->EvalAdd(t1, R);         // +2 = -2
        
        // T2: 010 = ¬L ∧ C ∧ ¬R
        auto t2 = cc->EvalAdd(nL, C);    // ¬L + C = 0
        t2 = cc->EvalAdd(t2, nR);         // -2
        
        // T3: 011 = ¬L ∧ C ∧ R (not needed, output 0)
        // Skip
        
        // T4: 100 = L ∧ ¬C ∧ ¬R
        auto t4 = cc->EvalAdd(L, nC);    // L + ¬C = 0
        t4 = cc->EvalAdd(t4, nR);         // -2
        
        // T5: 101 = L ∧ ¬C ∧ R
        auto t5 = cc->EvalAdd(L, nC);    // L + ¬C = 0
        t5 = cc->EvalAdd(t5, R);          // +2
        
        // T6: 110 = L ∧ C ∧ ¬R
        auto t6 = cc->EvalAdd(L, C);     // L + C = 4
        t6 = cc->EvalAdd(t6, nR);         // +2
        
        // ==========================================
        // OR via addition (approximation)
        // ==========================================
        
        // OR = max(T1, T2, T4, T5, T6)
        // Since we're using log space, OR ≈ EvalAdd with threshold
        auto result = cc->EvalAdd(t1, t2);
        result = cc->EvalAdd(result, t4);
        result = cc->EvalAdd(result, t5);
        result = cc->EvalAdd(result, t6);
        
        return result;
    };

    // ============================================
    // TRANSITION TABLE TEST
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE\n";
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
                int output = decrypt_log(ct_out);
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
