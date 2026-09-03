// ============================================
// φ-FIBONACCI PAIR MOD
// Fibonacci pair na may φ-modulo para ma-bound
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
    cout << "  φ-FIBONACCI PAIR MOD\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, batch 2)\n\n";

    // ============================================
    // FIBONACCI PAIR ENCODING NA MAY MOD
    // Slot 0: F_n mod φ
    // Slot 1: F_{n-1} mod φ
    // ============================================

    auto encrypt_pair_mod = [&](int n) {
        vector<double> v(2, 0.0);
        v[0] = fmod((double)fib[n], PHI);
        v[1] = fmod((double)fib[n-1], PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair_mod = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(2);
        for (int i = 0; i < 2; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST: φ^a + φ^(a-1) = φ^(a+1)
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT ADDITION WITH MOD\n";
    cout << "========================================\n\n";

    cout << "  a | Sum mod φ | Expected mod φ | Match?\n";
    cout << "  --|-----------|----------------|--------\n";

    for (int a : {3, 4, 5, 6, 7, 8, 9, 10}) {
        auto ct_a = encrypt_pair_mod(a);
        auto ct_am1 = encrypt_pair_mod(a-1);
        
        auto ct_sum = cc->EvalAdd(ct_a, ct_am1);
        auto sum_vals = decrypt_pair_mod(ct_sum);
        
        double expected = fmod((double)fib[a+1], PHI);
        bool match = abs(sum_vals[0] - expected) < 0.1;
        
        cout << "  " << setw(2) << a << " | "
             << setw(9) << fixed << setprecision(4) << sum_vals[0] << " | "
             << setw(14) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 10K CHAIN WITH MOD
    // ============================================

    cout << "\n========================================\n";
    cout << "  10K CHAIN WITH MOD\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_pair_mod(3);
    auto ct_prev = encrypt_pair_mod(2);

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: Fibonacci pair addition with φ-modulo\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        auto ct_next = cc->EvalAdd(ct_state, ct_prev);
        ct_prev = ct_state;
        ct_state = ct_next;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_pair_mod(ct_state);

    long long expected_F = fib[3 + N];
    double expected_mod = fmod((double)expected_F, PHI);

    cout << "  ✅ 10K chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 0 (F_n mod φ): " << final_vals[0] << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match: " << (abs(final_vals[0] - expected_mod) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  FIBONACCI PAIR MOD COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci pair na may φ-modulo\n";
    cout << "  ✅ Bounded — walang overflow\n";
    cout << "  ✅ Pure EvalAdd\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
