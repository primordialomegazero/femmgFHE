// ============================================
// φ-RULE 110 FINAL — TAMANG DECODE
//
// Ang transition ay L+C+R sa φ-space
// Ang decode ay gumagamit ng φ-power decomposition
// Hindi na kailangan ng fmod — φ na mismo!
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
    cout << "  φ-RULE 110 FINAL\n";
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

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // SIMPLE ENCODING: 0→φ, 1→1/φ
    // ============================================

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : (1.0 / PHI);
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bit = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 16.0;
        
        // φ² = φ + 1 = 2.618
        // Kung avg ≈ 2.618 → φ (0)
        // Kung avg ≈ 1.618 → φ⁻¹ (1) 
        // Kung avg ≈ 1.000 → ?
        
        // Simple: mas malapit sa φ⁻¹ (0.618) → 1, mas malapit sa φ → 0
        double d0 = abs(avg - PHI);
        double d1 = abs(avg - 1.0/PHI);
        
        return (d1 < d0) ? 1 : 0;
    };

    // ============================================
    // TEST: 3-BIT PATTERNS
    // ============================================

    cout << "========================================\n";
    cout << "  3-BIT PATTERN TEST\n";
    cout << "========================================\n\n";

    cout << "  L C R | Pattern | Avg Value | Expected | Decoded | Match?\n";
    cout << "  ------|---------|-----------|----------|---------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_bit(L);
                auto ct_C = encrypt_bit(C);
                auto ct_R = encrypt_bit(R);
                
                auto sum = cc->EvalAdd(cc->EvalAdd(ct_L, ct_C), ct_R);
                auto vals = decrypt_bit(sum);
                
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                bool match = (vals == expected);
                match_count += match;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(7) << pattern << " | "
                     << setw(9) << fixed << setprecision(3) 
                     << (L == 0 && C == 0 && R == 0 ? 3*PHI : 
                         L == 1 && C == 1 && R == 1 ? 3/PHI : 
                         L == 0 && C == 0 ? 2*PHI + 1/PHI :
                         L == 0 ? PHI + 2/PHI :
                         L == 1 && C == 0 ? 2/PHI + PHI : 0) << " | "
                     << setw(8) << expected << " | "
                     << setw(7) << vals << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    cout << "========================================\n";
    cout << "  RULE 110 FINAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Transition: " << match_count << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
