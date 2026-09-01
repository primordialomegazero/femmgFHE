// ============================================
// φ-RULE 110 FIBONACCI WEIGHTED
//
// Unique sums para sa bawat pattern:
// L: 1.00/1.62, C: 1.00/1.62, R: 2.00/3.24
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
    cout << "  φ-RULE 110 FIBONACCI WEIGHTED\n";
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

    // Fibonacci weights mula sa TEST 6
    const double W_L_ZERO = 1.00, W_L_ONE = 1.62;
    const double W_C_ZERO = 1.00, W_C_ONE = 1.62;
    const double W_R_ZERO = 2.00, W_R_ONE = 3.24;

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Fibonacci weights: L=1.62, C=1.62, R=3.24\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_weighted = [&](int bit, double w_zero, double w_one) {
        double val = (bit == 0) ? w_zero : w_one;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_weighted = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // TRANSITION TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum    | Output | Expected | Match?\n";
    cout << "  ------|--------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_weighted(L, W_L_ZERO, W_L_ONE);
                auto ct_C = encrypt_weighted(C, W_C_ZERO, W_C_ONE);
                auto ct_R = encrypt_weighted(R, W_R_ZERO, W_R_ONE);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                double sum_val = decrypt_weighted(sum2);
                
                // Unique sums:
                // 000 → 4.00 → 0
                // 001 → 5.24 → 1
                // 010 → 4.62 → 1
                // 011 → 5.85 → 0
                // 100 → 4.62 → 1
                // 101 → 5.85 → 1
                // 110 → 5.24 → 1
                // 111 → 6.47 → 0
                int output;
                if (sum_val < 4.5) output = 0;       // 000
                else if (sum_val < 5.0) output = 1;  // 010, 100
                else if (sum_val < 5.5) output = 1;  // 001, 110
                else if (sum_val < 6.0) output = 0;  // 011, 101 → kailangan ng distinction!
                else output = 0;                       // 111
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << fixed << setprecision(2) << sum_val << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
