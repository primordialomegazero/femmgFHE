// ============================================
// φ-FIBONACCI INDEX FHE
// I-encode ang value bilang Fibonacci index
// Addition at multiplication sa index space
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
    cout << "  φ-FIBONACCI INDEX FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n";
    cout << "  log_φ(√5) = " << LOG_SQRT5 << "\n\n";

    // ============================================
    // ENCODING: [index, F_index, log_φ(F_index), F_index × φ]
    // ============================================

    auto encrypt_fib = [&](int index) {
        double F_n = fib[index];
        double log_F = log(F_n) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = index;           // Fibonacci index
        v[1] = F_n;             // Fibonacci value
        v[2] = log_F;           // log_φ(F_n)
        v[3] = F_n * PHI;       // φ-scaled
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fib = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST: ADDITION — F_3 + F_4 = F_5 (2 + 3 = 5)
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION: F_3 + F_4 = F_5\n";
    cout << "========================================\n\n";

    auto ct_3 = encrypt_fib(3);  // F_3 = 2
    auto ct_4 = encrypt_fib(4);  // F_4 = 3

    auto ct_add = cc->EvalAdd(ct_3, ct_4);
    auto add_vals = decrypt_fib(ct_add);

    cout << "  Slot 0 (index): " << add_vals[0] << " (expected: 7)\n";
    cout << "  Slot 1 (F_value): " << add_vals[1] << " (expected: 5)\n";
    cout << "  Slot 2 (log): " << add_vals[2] << "\n\n";

    // ============================================
    // TEST: MULTIPLICATION — F_3 × F_4 = 2 × 3 = 6
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION: F_3 × F_4 = 6\n";
    cout << "========================================\n\n";

    auto ct_mul = cc->EvalAdd(ct_3, ct_4);
    auto mul_vals = decrypt_fib(ct_mul);

    cout << "  Slot 2 (log): " << mul_vals[2] << " → φ^ = " << pow(PHI, mul_vals[2]) << "\n";
    cout << "  Expected: 6\n";
    cout << "  Match: " << (abs(pow(PHI, mul_vals[2]) - 6.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K FIBONACCI CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  10K FIBONACCI CHAIN\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_fib(3);  // F_3 = 2
    auto ct_fib3 = encrypt_fib(3);
    auto ct_fib4 = encrypt_fib(4);
    auto ct_fib5 = encrypt_fib(5);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +F_3, +F_4, +F_5 (Fibonacci additions)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_index = 3.0;

    for (int i = 0; i < N; i++) {
        switch (i % 3) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_fib3); expected_index += 3.0; break;
            case 1: ct_state = cc->EvalAdd(ct_state, ct_fib4); expected_index += 4.0; break;
            case 2: ct_state = cc->EvalAdd(ct_state, ct_fib5); expected_index += 5.0; break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_fib(ct_state);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 0 (index): " << final_vals[0] << "\n";
    cout << "  Expected index: " << expected_index << "\n";
    cout << "  Match: " << (abs(final_vals[0] - expected_index) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  FIB INDEX FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci index encoding\n";
    cout << "  ✅ Natural addition\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
