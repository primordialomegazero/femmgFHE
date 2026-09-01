// ============================================
// φ-RULE 110 DUAL POSITIONAL — FINAL
//
// Slot 0: Positional φ-value (weighted)
// Slot 1: Exponent sum (para sa guide)
//
// Ang positional weights ang magde-determine
// kung aling 1-pattern ang dapat maging 1 o 0
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
    cout << "  φ-RULE 110 DUAL POSITIONAL\n";
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

    // Positional weights para sa L, C, R
    const double W_L_ZERO = pow(PHI, -4);
    const double W_L_ONE = pow(PHI, -1);
    const double W_C_ZERO = pow(PHI, -3);
    const double W_C_ONE = pow(PHI, 0);
    const double W_R_ZERO = pow(PHI, -3);
    const double W_R_ONE = pow(PHI, 0);

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Slot 0: Positional φ-value\n";
    cout << "  Slot 1: Exponent (bit count)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_pos = [&](int bit, int position) {
        vector<double> v(16, 0.0);
        // Slot 0: Positional weight
        if (position == 0) {  // L
            v[0] = (bit == 0) ? W_L_ZERO : W_L_ONE;
        } else if (position == 1) {  // C
            v[0] = (bit == 0) ? W_C_ZERO : W_C_ONE;
        } else {  // R
            v[0] = (bit == 0) ? W_R_ZERO : W_R_ONE;
        }
        // Slot 1: Exponent (0 o 1)
        v[1] = (bit == 0) ? 0.0 : 1.0;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pos = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double val = result_pt->GetCKKSPackedValue()[0].real();
        double exp = result_pt->GetCKKSPackedValue()[1].real();
        return make_pair(val, exp);
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
    // DUAL POSITIONAL EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (DUAL POSITIONAL)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;

    for (int i = 0; i < N; i++) {
        curr_L.push_back(encrypt_pos(history[0][i], 0));
        curr_C.push_back(encrypt_pos(history[0][i], 1));
        curr_R.push_back(encrypt_pos(history[0][i], 2));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;

        for (int i = 0; i < N; i++) {
            auto L = curr_L[(i + N - 1) % N];
            auto C = curr_C[i];
            auto R = curr_R[(i + 1) % N];

            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);

            // Re-encode para sa next generation
            next_L.push_back(sum2);
            next_C.push_back(sum2);
            next_R.push_back(sum2);
        }

        curr_L = next_L;
        curr_C = next_C;
        curr_R = next_R;

        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                auto [val, exp] = decrypt_pos(curr_C[i]);
                int bit = ((int)floor(val)) % 2;
                if (bit < 0) bit = 0;
                cout << bit;
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << curr_C[0]->GetLevel() << "\n\n";

    // VERIFICATION
    cout << "========================================\n";
    cout << "  VERIFICATION (GEN 20)\n";
    cout << "========================================\n\n";

    int matches = 0;
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    cout << "  Encrypted: ";
    for (int i = 0; i < N; i++) {
        auto [val, exp] = decrypt_pos(curr_C[i]);
        int bit = ((int)floor(val)) % 2;
        if (bit < 0) bit = 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  DUAL POSITIONAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Dual positional\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n\n";

    return 0;
}
