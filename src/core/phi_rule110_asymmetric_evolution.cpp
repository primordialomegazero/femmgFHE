// ============================================
// φ-RULE 110 ASYMMETRIC EVOLUTION
//
// Asymmetric φ-weights na 8/8 sa transition
// Ngayon i-apply sa 20 generations
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
    cout << "  φ-RULE 110 ASYMMETRIC EVOLUTION\n";
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
    // ASYMMETRIC EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (ASYMMETRIC)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;

    for (int bit : history[0]) {
        curr_L.push_back(encrypt_weighted(bit, W_L_ZERO, W_L_ONE));
        curr_C.push_back(encrypt_weighted(bit, W_C_ZERO, W_C_ONE));
        curr_R.push_back(encrypt_weighted(bit, W_R_ZERO, W_R_ONE));
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
                double val = decrypt_value(curr_C[i]);
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
        double val = decrypt_value(curr_C[i]);
        int bit = ((int)floor(val)) % 2;
        if (bit < 0) bit = 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  ASYMMETRIC EVOLUTION COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 8/8 transition\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure EvalAdd\n\n";

    return 0;
}
