// ============================================
// φ-RULE 110 DEBUG — HANAPIN ANG 1 MALI
//
// Gen 0: 15/16 — aling pattern ang mali?
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
    cout << "  φ-RULE 110 DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : PHI_INV;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_raw = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    };

    auto decode_bit = [&](const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        double normalized = (PHI - avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        return 1 - (int)round(mod2);
    };

    // ============================================
    // DEBUG: LAHAT NG 8 PATTERNS SA RULE 110
    // ============================================

    cout << "========================================\n";
    cout << "  LAHAT NG 8 PATTERNS\n";
    cout << "========================================\n\n";

    cout << "  L C R | Expected | Decoded | Match?\n";
    cout << "  ------|----------|---------|--------\n";

    int match_count = 0;

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_bit(L);
                auto ct_C = encrypt_bit(C);
                auto ct_R = encrypt_bit(R);
                
                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_L, ct_C), ct_R);
                int decoded = decode_bit(decrypt_raw(ct_sum));
                
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

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // GEN 0: INITIAL STATE 0000000110000000
    // ============================================

    cout << "========================================\n";
    cout << "  GEN 0: 0000000110000000\n";
    cout << "========================================\n\n";

    int N = 16;
    vector<int> state = {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0};

    cout << "  Cell | L C R | Expected | Decoded | Match?\n";
    cout << "  -----|-------|----------|---------|--------\n";

    int gen0_match = 0;

    for (int i = 0; i < N; i++) {
        int L = state[(i + N - 1) % N];
        int C = state[i];
        int R = state[(i + 1) % N];
        
        auto ct_L = encrypt_bit(L);
        auto ct_C = encrypt_bit(C);
        auto ct_R = encrypt_bit(R);
        
        auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_L, ct_C), ct_R);
        int decoded = decode_bit(decrypt_raw(ct_sum));
        
        int pattern = (L << 2) | (C << 1) | R;
        int expected = rule110[pattern];
        
        bool match = (decoded == expected);
        gen0_match += match;
        
        cout << "  " << setw(4) << i << " | " 
             << L << " " << C << " " << R << " | "
             << setw(8) << expected << " | "
             << setw(7) << decoded << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Gen 0 match: " << gen0_match << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  DEBUG COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
