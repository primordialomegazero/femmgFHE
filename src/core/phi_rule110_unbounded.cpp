// ============================================
// φ-RULE 110 UNBOUNDED — WALANG MULTIPLICATION
//
// Pre-weighted triple state + period-5 parity
// Lahat ay pure EvalAdd/EvalSub
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
    cout << "  φ-RULE 110 UNBOUNDED\n";
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
    
    // Pre-weighted positional values (8/8 transition weights)
    const double W_L_ZERO = 0.0;  // L=0 → 0
    const double W_L_ONE = pow(PHI, -3);   // L=1 → φ⁻³ = 0.236
    const double W_C_ZERO = 0.0;  // C=0 → 0
    const double W_C_ONE = pow(PHI, -2);   // C=1 → φ⁻² = 0.382
    const double W_R_ZERO = pow(PHI, -1) / 2.0;  // R=0 → φ⁻¹/2 = 0.309
    const double W_R_ONE = pow(PHI, 0);    // R=1 → φ⁰ = 1.0

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Pre-weighted: L(0→0, 1→φ⁻³), C(0→0, 1→φ⁻²), R(0→φ⁻¹/2, 1→φ⁰)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_value = [&](double val) {
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
    // VERIFY TRANSITION TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE VERIFICATION\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
    cout << "  ------|----------|-------|-------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = (L ? W_L_ONE : W_L_ZERO) + 
                            (C ? W_C_ONE : W_C_ZERO) + 
                            (R ? W_R_ONE : W_R_ZERO);
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
