// ============================================
// φ-RULE 110 MODSIZE TEST — IBA'T IBANG MODSIZE
//
// Test: 50 vs 59 vs 60 vs 80
// Hanapin ang tamang precision para sa φ-transition
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
    cout << "  φ-RULE 110 MODSIZE TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Subukan ang iba't ibang modsize
    vector<uint32_t> modsizes = {50, 55, 59, 60, 80};

    for (uint32_t modsize : modsizes) {
        cout << "========================================\n";
        cout << "  MODSIZE: " << modsize << "\n";
        cout << "========================================\n\n";

        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(0);
        parameters.SetScalingModSize(modsize);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_128_classic);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);

        auto encrypt_weighted = [&](int bit, double weight) {
            double val = bit ? weight : 0.0;
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

        int match_count = 0;
        cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
        cout << "  ------|----------|-------|-------|----------\n";

        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    auto ct_L = encrypt_weighted(L, W_L_ONE);
                    auto ct_C = encrypt_weighted(C, W_C_ONE);
                    auto ct_R = (R == 1) ? encrypt_weighted(1, W_R_ONE) : encrypt_weighted(1, W_R_ZERO);
                    
                    auto sum1 = cc->EvalAdd(ct_L, ct_C);
                    auto sum2 = cc->EvalAdd(sum1, ct_R);
                    
                    double sum_val = decrypt_value(sum2);
                    int floor_val = (int)floor(sum_val);
                    int mod2 = floor_val % 2;
                    int expected = rule110[(L << 2) | (C << 1) | R];
                    bool match = (mod2 == expected);
                    if (match) match_count++;
                    
                    cout << "  " << L << " " << C << " " << R << " | "
                         << setw(8) << fixed << setprecision(6) << sum_val << " | "
                         << setw(5) << floor_val << " | "
                         << setw(5) << mod2 << " | "
                         << setw(8) << expected << " | "
                         << (match ? "✅" : "❌") << "\n";
                }
            }
        }
        
        cout << "\n  Modsize " << modsize << " → Match: " << match_count << "/8\n\n";
    }

    return 0;
}
