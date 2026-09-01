// ============================================
// φ-RULE 110 QUADRATIC NAND — EXACT FORMULA
//
// Rule 110 = L + C + R - 2LC - LR + 2CR
// (sa log space, may quadratic correction)
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
    cout << "  φ-RULE 110 QUADRATIC NAND\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Quadratic formula: L + C + R - 2LC - LR + 2CR\n\n";

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
    // RULE 110 EXACT QUADRATIC
    // ============================================
    
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        // Linear: L + C + R
        auto linear = cc->EvalAdd(L, C);
        linear = cc->EvalAdd(linear, R);
        
        // Quadratic: -2LC
        auto LC = cc->EvalMult(L, C);
        auto neg_2LC = cc->EvalMult(LC, -2.0);
        
        // Quadratic: -LR
        auto LR = cc->EvalMult(L, R);
        auto neg_LR = cc->EvalNegate(LR);
        
        // Quadratic: +2CR
        auto CR = cc->EvalMult(C, R);
        auto pos_2CR = cc->EvalMult(CR, 2.0);
        
        // Result = linear + quadratic corrections
        auto result = cc->EvalAdd(linear, neg_2LC);
        result = cc->EvalAdd(result, neg_LR);
        result = cc->EvalAdd(result, pos_2CR);
        
        return result;
    };

    // ============================================
    // TRANSITION TABLE TEST
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (QUADRATIC)\n";
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
