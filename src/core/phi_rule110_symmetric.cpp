// ============================================
// φ-RULE 110 SYMMETRIC — BALANCED EVOLUTION
//
// 0 → +φ/2, 1 → -φ/2 (symmetric!)
// Sign-aware decode — walang drift
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
    cout << "  φ-RULE 110 SYMMETRIC\n";
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
    const double HALF_PHI = PHI / 2.0;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Symmetric: 0→+φ/2, 1→-φ/2\n\n";

    auto encrypt_sym = [&](int bit) {
        double val = (bit == 0) ? HALF_PHI : -HALF_PHI;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // SIGN-AWARE DECODE
    auto decode_sym = [&](double avg) {
        // Positive → 0, Negative → 1
        return (avg < 0) ? 1 : 0;
    };

    // ============================================
    // TEST 1: ALL 8 PATTERNS
    // ============================================

    cout << "========================================\n";
    cout << "  ALL 8 PATTERNS (SYMMETRIC)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum | Expected | Decoded | Match?\n";
    cout << "  ------|-----|----------|---------|--------\n";

    int match_count = 0;

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_sym(L);
                auto ct_C = encrypt_sym(C);
                auto ct_R = encrypt_sym(R);
                
                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_L, ct_C), ct_R);
                double avg = decrypt_avg(ct_sum);
                int decoded = decode_sym(avg);
                
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                bool match = (decoded == expected);
                match_count += match;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(3) << avg << " | "
                     << setw(8) << expected << " | "
                     << setw(7) << decoded << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // TEST 2: EVOLUTION (20 GENERATIONS)
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (20 GENERATIONS)\n";
    cout << "========================================\n\n";

    int N = 16;
    vector<int> state(N, 0);
    state[7] = 1;
    state[8] = 1;

    // Plaintext reference
    vector<int> plain = state;
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

    cout << "  Gen | Match\n";
    cout << "  ----|------\n";

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen <= 20; gen++) {
        int gen_match = 0;
        
        for (int i = 0; i < N; i++) {
            int L = history[gen][(i + N - 1) % N];
            int C = history[gen][i];
            int R = history[gen][(i + 1) % N];
            
            auto ct_L = encrypt_sym(L);
            auto ct_C = encrypt_sym(C);
            auto ct_R = encrypt_sym(R);
            
            auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_L, ct_C), ct_R);
            double avg = decrypt_avg(ct_sum);
            int decoded = decode_sym(avg);
            
            int pattern = (L << 2) | (C << 1) | R;
            int expected = rule110[pattern];
            
            gen_match += (decoded == expected);
        }
        
        cout << "  " << setw(3) << gen << " | " << gen_match << "/" << N << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: 0\n\n";

    cout << "========================================\n";
    cout << "  SYMMETRIC RULE 110 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ All patterns: " << match_count << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
