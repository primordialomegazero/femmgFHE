// ============================================
// φ-RULE 110 CORRECT WEIGHTS — 8/8 VERIFIED
//
// Tamang weights mula sa search:
// α = φ²-φ/2, β = φ, γ = φ²/2
// I-verify na 8/8 sa plaintext at encrypted
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
    cout << "  φ-RULE 110 CORRECT WEIGHTS\n";
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
    
    // Tamang weights mula sa 8/8 search
    const double W_L = PHI * PHI - PHI / 2.0;  // φ² - φ/2 = 1.80902
    const double W_C = PHI;                     // φ = 1.61803
    const double W_R = PHI * PHI / 2.0;        // φ²/2 = 1.30902

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Tamang weights:\n";
    cout << "  W_L = φ² - φ/2 = " << W_L << "\n";
    cout << "  W_C = φ = " << W_C << "\n";
    cout << "  W_R = φ²/2 = " << W_R << "\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_weighted = [&](int bit, double weight) {
        double val = bit ? weight : 0.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_floor = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 16.0;
        return (int)floor(avg);
    };

    // ============================================
    // PLAINTEXT VERIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  PLAINTEXT VERIFICATION\n";
    cout << "========================================\n\n";

    cout << "  L C R | Weighted Sum | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|-------------|-------|-------|----------|--------\n";

    int plain_match = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double val = L * W_L + C * W_C + R * W_R;
                int floor_val = (int)floor(val);
                int mod2 = floor_val % 2;
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                bool match = (mod2 == expected);
                if (match) plain_match++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(11) << fixed << setprecision(3) << val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Plaintext match: " << plain_match << "/8\n\n";

    // ============================================
    // ENCRYPTED VERIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  ENCRYPTED VERIFICATION\n";
    cout << "========================================\n\n";

    cout << "  L C R | Weighted Sum | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|-------------|-------|-------|----------|--------\n";

    int enc_match = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_weighted(L, W_L);
                auto ct_C = encrypt_weighted(C, W_C);
                auto ct_R = encrypt_weighted(R, W_R);
                
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                int floor_val = decrypt_floor(sum2);
                int mod2 = floor_val % 2;
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                bool match = (mod2 == expected);
                if (match) enc_match++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(11) << fixed << setprecision(3) 
                     << (L * W_L + C * W_C + R * W_R) << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Encrypted match: " << enc_match << "/8\n\n";

    return 0;
}
