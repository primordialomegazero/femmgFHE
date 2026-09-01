// ============================================
// φ-RULE 110 ASYMMETRIC NAND
//
// Asymmetric φ-weights para ma-distinguish
// ang L, C, R positions:
// L: 0 → φ⁻⁴, 1 → φ⁻¹
// C: 0 → φ⁻³, 1 → φ⁰
// R: 0 → φ⁻³, 1 → φ⁰
//
// Transition: sum = w_L·L + w_C·C + w_R·R
// Output: floor(sum) mod 2
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
    cout << "  φ-RULE 110 ASYMMETRIC NAND\n";
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

    const double W_L_ZERO = pow(PHI, -4);
    const double W_L_ONE = pow(PHI, -1);
    const double W_C_ZERO = pow(PHI, -3);
    const double W_C_ONE = pow(PHI, 0);
    const double W_R_ZERO = pow(PHI, -3);
    const double W_R_ONE = pow(PHI, 0);

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Asymmetric weights: L≠C≠R\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_weighted = [&](int bit, double w_zero, double w_one) {
        double val = (bit == 0) ? w_zero : w_one;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
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
    cout << "  TRANSITION TABLE (ASYMMETRIC)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_weighted(L, W_L_ZERO, W_L_ONE);
                auto ct_C = encrypt_weighted(C, W_C_ZERO, W_C_ONE);
                auto ct_R = encrypt_weighted(R, W_R_ZERO, W_R_ONE);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                double val = decrypt_value(sum2);
                int output = ((int)floor(val)) % 2;
                if (output < 0) output = 0;
                
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
