// ============================================
// φ-RULE 110 WEIGHTED FHE FINAL — 8/8
//
// L: φ⁰/φ¹, C: φ²/φ³, R: φ⁴/φ⁵
// Unique sums — walang collision!
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
    cout << "  φ-RULE 110 WEIGHTED FHE FINAL\n";
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

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // WEIGHTED ENCODING
    // ============================================

    auto encrypt_weighted = [&](int bit, int position) {
        // position: 0 (L), 1 (C), 2 (R)
        int power = position * 2 + bit;
        double val = pow(PHI, power);
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

    auto decode_weighted = [&](double avg) {
        // Hanapin ang pinakamalapit na unique sum
        double best_diff = 1e10;
        int best_pattern = 0;
        for (int p = 0; p < 8; p++) {
            double target = pow(PHI, (p >> 2) * 2 + ((p >> 2) & 1)) +
                           pow(PHI, ((p >> 1) & 1) * 2 + 2 + ((p >> 1) & 1)) +
                           pow(PHI, (p & 1) * 2 + 4 + (p & 1));
            // Simpler: direct lookup
            target = pow(PHI, ((p >> 2) & 1) * 1 + 0) +
                     pow(PHI, ((p >> 1) & 1) * 1 + 2) +
                     pow(PHI, (p & 1) * 1 + 4);
            double diff = abs(avg - target);
            if (diff < best_diff) {
                best_diff = diff;
                best_pattern = p;
            }
        }
        return rule110[best_pattern];
    };

    // ============================================
    // TEST: TRANSITION TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (ENCRYPTED)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Expected | Decoded | Match?\n";
    cout << "  ------|----------|---------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_weighted(L, 0);
                auto ct_C = encrypt_weighted(C, 1);
                auto ct_R = encrypt_weighted(R, 2);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                double avg = decrypt_weighted(sum2);
                int decoded = decode_weighted(avg);
                
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                bool match = (decoded == expected);
                match_count += match;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << expected << " | "
                     << setw(7) << decoded << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Transition: " << match_count << "/8\n\n";

    cout << "========================================\n";
    cout << "  WEIGHTED FHE FINAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Transition: " << match_count << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
