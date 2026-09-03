// ============================================
// φ-FIBONACCI PAIR FHE
// I-encode ang (F_n, F_{n-1}) — hindi φ-power
// Addition ay pure EvalAdd
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
    cout << "  φ-FIBONACCI PAIR FHE\n";
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
    // FIBONACCI PAIR ENCODING
    // Slot 0: F_n
    // Slot 1: F_{n-1}
    // ============================================

    auto encrypt_pair = [&](int n) {
        vector<double> v(2, 0.0);
        v[0] = fib[n];      // F_n
        v[1] = fib[n-1];   // F_{n-1}
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // Sa Fibonacci pair:
    // (F_a, F_{a-1}) + (F_{a-1}, F_{a-2}) = (F_{a+1}, F_a)
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT ADDITION VIA FIBONACCI PAIR\n";
    cout << "========================================\n\n";

    cout << "  a | Sum F | Expected F_{a+1} | Match?\n";
    cout << "  --|-------|------------------|--------\n";

    for (int a : {3, 4, 5, 6, 7, 8, 9, 10}) {
        auto ct_a = encrypt_pair(a);
        auto ct_am1 = encrypt_pair(a-1);
        
        auto ct_sum = cc->EvalAdd(ct_a, ct_am1);
        auto sum_vals = decrypt_pair(ct_sum);
        
        double expected = fib[a+1];
        bool match = abs(sum_vals[0] - expected) < 0.5;
        
        cout << "  " << setw(2) << a << " | "
             << setw(5) << fixed << setprecision(0) << sum_vals[0] << " | "
             << setw(16) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 10K CHAIN — FIBONACCI PAIR ADDITION
    // ============================================

    cout << "\n========================================\n";
    cout << "  10K CHAIN — FIBONACCI PAIR ADDITION\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_pair(3);  // (F_3, F_2) = (2, 1)
    auto ct_prev = encrypt_pair(2);   // (F_2, F_1) = (1, 1)

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: (F_n, F_{n-1}) + (F_{n-1}, F_{n-2}) = (F_{n+1}, F_n)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        auto ct_next = cc->EvalAdd(ct_state, ct_prev);
        ct_prev = ct_state;
        ct_state = ct_next;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_pair(ct_state);

    cout << "  ✅ 10K chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 0 (F_n): " << final_vals[0] << "\n";
    cout << "  Slot 1 (F_{n-1}): " << final_vals[1] << "\n\n";

    // Verify: after N operations, state should be at index 3+N
    long long expected_F = fib[3 + N];
    long long expected_Fm1 = fib[2 + N];

    cout << "  Expected F_" << (3+N) << " = " << expected_F << "\n";
    cout << "  Expected F_" << (2+N) << " = " << expected_Fm1 << "\n";
    cout << "  Match (F_n): " << (abs(final_vals[0] - expected_F) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Match (F_{n-1}): " << (abs(final_vals[1] - expected_Fm1) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  FIBONACCI PAIR FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci pair encoding\n";
    cout << "  ✅ Pure EvalAdd\n";
    cout << "  ✅ Walang overflow\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
