// ============================================
// φ-RULE 110 LOOKUP NAND — DIRECT PATTERN
//
// Direct lookup table sa log space
// Encoding: 0 → -2, 1 → +2
//
// Rule 110 truth table:
// 000→0, 001→1, 010→1, 011→0,
// 100→1, 101→1, 110→1, 111→0
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
    cout << "  φ-RULE 110 LOOKUP NAND\n";
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
    cout << "  Log space: 0→-2, 1→+2\n\n";

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
    // RULE 110 DIRECT — PATTERN WEIGHTS
    // ============================================
    
    // Para sa bawat pattern, may weight:
    // 000: -1, 001: +1, 010: +1, 011: -1,
    // 100: +1, 101: +1, 110: +1, 111: -1
    
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        // Direct sum: L + C + R
        auto sum = cc->EvalAdd(L, C);
        sum = cc->EvalAdd(sum, R);
        
        // Sa log space:
        // sum = -6 (000) → output 0
        // sum = -2 (001, 010, 100) → output 1 for 001,010; 0 for 100
        // sum = +2 (011, 101, 110) → output 0 for 011; 1 for 101,110
        // sum = +6 (111) → output 0
        
        // Kaya kailangan natin ng second-order correction
        // Correction: L AND C (negative) + L AND R (positive)
        
        // L*C term: kung L=1 at C=1, mag-aadjust tayo
        auto LC = cc->EvalAdd(L, C);  // L+C: -4,0,0,+4
        // L*R term
        auto LR = cc->EvalAdd(L, R);  // L+R: -4,0,0,+4
        
        // Correction = -(LC) + (LR)
        auto correction = cc->EvalNegate(LC);
        correction = cc->EvalAdd(correction, LR);
        
        // Final = sum + correction
        return cc->EvalAdd(sum, correction);
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
