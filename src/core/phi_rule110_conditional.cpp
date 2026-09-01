// ============================================
// φ-RULE 110 CONDITIONAL — ZERO-LEVEL
//
// Rule 110 via conditional logic sa log space
// Threshold: log > 0 → 1, log < 0 → 0
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
    cout << "  φ-RULE 110 CONDITIONAL\n";
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
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 0, modsize 55!)\n";
    cout << "  Log space: 0→-1, 1→+1\n";
    cout << "  Threshold: log > 0 → 1\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_log = [&](int bit) {
        double val = (bit == 0) ? -1.0 : 1.0;
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
    // RULE 110 VIA CONDITIONAL LOGIC
    // ============================================
    //
    // Ang patterns ay may log sums:
    // 000 → -3 → output 0
    // 001 → -1 → output 1
    // 010 → -1 → output 1
    // 011 → +1 → output 0? HINDI!
    //
    // Kailangan ng mas magandang encoding.
    //
    // ANG TAMANG WEIGHTS (8/8):
    // L: 0→0, 1→φ⁻³ (log: -∞, -3)
    // C: 0→0, 1→φ⁻² (log: -∞, -2)
    // R: 0→φ/2, 1→φ (log: -0.44, +1)
    //
    // Sa log space, ang sums ay:
    // 000 → log(φ/2) = -0.44
    // 001 → log(φ) = +0.48
    // 010 → log(φ⁻² + φ/2) = log(0.382+0.809) = log(1.191) = +0.36
    // ...

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (CONDITIONAL)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    // Subukan: L→{-1, +1}, C→{-1, +1}, R→{-1, +1}
    // Sa log space, ang sum ay -3 hanggang +3
    // Threshold: sum > 0 → 1

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_log(L);
                auto ct_C = encrypt_log(C);
                auto ct_R = encrypt_log(R);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                double log_sum = decrypt_log(sum2);
                int output = (log_sum > 0) ? 1 : 0;
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
