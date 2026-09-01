// ============================================
// φ-RULE 110 EMERGENT BFV — DIRETSO NA
//
// Hindi computation—DIRETSO NA sa φ-weights.
// Ang φ-emergent properties ang magbibigay
// ng tamang transition nang walang lookup.
//
// Asymmetric weights (mula sa 8/8 CKKS test):
// L: 0→0, 1→φ⁻³
// C: 0→0, 1→φ⁻²
// R: 0→φ/2, 1→φ
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
    cout << "  φ-RULE 110 EMERGENT BFV\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(0);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;

    // SCALE FACTOR para ma-convert φ sa integer
    const int64_t SCALE = 10000;

    // Asymmetric weights (from 8/8 CKKS):
    const int64_t W_L_ZERO = 0;
    const int64_t W_L_ONE = (int64_t)(pow(PHI, -3) * SCALE);  // 0.236 × 10000 = 2360
    const int64_t W_C_ZERO = 0;
    const int64_t W_C_ONE = (int64_t)(pow(PHI, -2) * SCALE);  // 0.382 × 10000 = 3820
    const int64_t W_R_ZERO = (int64_t)(PHI / 2.0 * SCALE);     // 0.809 × 10000 = 8090
    const int64_t W_R_ONE = (int64_t)(PHI * SCALE);             // 1.618 × 10000 = 16180

    cout << "  ✅ BFV initialized (exact integers!)\n";
    cout << "  φ-Emergent weights (scaled):\n";
    cout << "  L: 0→" << W_L_ZERO << ", 1→" << W_L_ONE << "\n";
    cout << "  C: 0→" << W_C_ZERO << ", 1→" << W_C_ONE << "\n";
    cout << "  R: 0→" << W_R_ZERO << ", 1→" << W_R_ONE << "\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_weighted = [&](int bit, int64_t w_zero, int64_t w_one) {
        int64_t val = (bit == 0) ? w_zero : w_one;
        vector<int64_t> v(1, val);
        Plaintext pt = cc->MakePackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_weighted = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetPackedValue()[0];
    };

    // ============================================
    // TRANSITION TABLE (EMERGENT)
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (EMERGENT)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum     | Output | Expected | Match?\n";
    cout << "  ------|---------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_weighted(L, W_L_ZERO, W_L_ONE);
                auto ct_C = encrypt_weighted(C, W_C_ZERO, W_C_ONE);
                auto ct_R = encrypt_weighted(R, W_R_ZERO, W_R_ONE);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                int64_t sum_val = decrypt_weighted(sum2);
                
                // Emergent threshold: floor(sum / SCALE) mod 2
                int output = ((sum_val / SCALE) % 2 + 2) % 2;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(7) << sum_val << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
