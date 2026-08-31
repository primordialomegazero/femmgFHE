// ============================================
// φ-RULE 110 EXPONENT — φ-EXPONENT STATE
//
// I-store ang φ-exponent bilang encrypted state
// Ang parity ay period-5: 1, 1, 0, 0, 0
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
    cout << "  φ-RULE 110 EXPONENT\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    
    // State exponents: 0 → -5, 1 → -2
    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;
    
    // Positional offsets
    const double OFFSET_L = 1.0;
    const double OFFSET_C = 2.0;
    const double OFFSET_R = 2.0;

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  State exponents: 0→-5, 1→-2\n";
    cout << "  Offsets: L=+1, C=+2, R=+2\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_exp = [&](double exp_val) {
        vector<double> v(16, exp_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_exp = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // PLAINTEXT REFERENCE
    // ============================================

    int N = 16;
    vector<int> plain(N, 0);
    plain[7] = 1;
    plain[8] = 1;

    vector<vector<int>> history;
    history.push_back(plain);
    for (int gen = 0; gen < 20; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = plain[(i + N - 1) % N];
            int C = plain[i];
            int R = plain[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        plain = next;
        history.push_back(plain);
    }

    // ============================================
    // VERIFY: EXPONENT TRANSITION
    // ============================================

    cout << "========================================\n";
    cout << "  EXPONENT TRANSITION VERIFICATION\n";
    cout << "========================================\n\n";

    cout << "  L C R | φ^sum   | φ-log  | Period-5 | Parity | Expected\n";
    cout << "  ------|---------|--------|----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                // φ-values
                double l_val = (L ? pow(PHI, EXP_ONE + OFFSET_L) : pow(PHI, EXP_ZERO + OFFSET_L));
                double c_val = (C ? pow(PHI, EXP_ONE + OFFSET_C) : pow(PHI, EXP_ZERO + OFFSET_C));
                double r_val = (R ? pow(PHI, EXP_ONE + OFFSET_R) : pow(PHI, EXP_ZERO + OFFSET_R));
                double sum = l_val + c_val + r_val;
                
                // Rotation: sum - φ⁻¹
                double rotated = sum - pow(PHI, -1);
                
                // φ-log ng rotated
                double log_rotated = log(rotated) / log(PHI);
                int nearest_exp = (int)round(log_rotated);
                int period5 = ((nearest_exp % 5) + 5) % 5;
                int parity = (period5 == 0 || period5 == 1) ? 1 : 0;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (parity == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << rotated << " | "
                     << setw(6) << fixed << setprecision(2) << log_rotated << " | "
                     << setw(8) << period5 << " | "
                     << setw(6) << parity << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
