// ============================================
// φ-10K FIBONACCI MODULO
// 10K arbitrary chains na may Fibonacci modulo
// para ma-bound ang log space
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
    cout << "  φ-10K FIBONACCI MODULO\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 60; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING NA MAY FIBONACCI MODULO
    // Slot 0: F_{n-1} mod φ
    // Slot 1: F_n mod φ
    // Slot 2: log_φ(x)
    // Slot 3: φ^frac
    // ============================================

    auto encrypt_fib = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        
        vector<double> v(4, 0.0);
        v[0] = fmod((double)fib[n-1], PHI);
        v[1] = fmod((double)fib[n], PHI);
        v[2] = log_phi_x;
        v[3] = phi_frac;
        
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
    // TEST: 5 × 7 = 35
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 5 × 7 = 35\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_fib(5.0);
    auto ct_7 = encrypt_fib(7.0);

    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_fib(ct_mult);

    double log_res = mult_vals[2];
    double result = pow(PHI, log_res);
    double mod_phi = fmod(result, PHI);

    cout << "  Slot 2 (log): " << log_res << "\n";
    cout << "  φ^(log): " << result << "\n";
    cout << "  φ^(log) mod φ: " << mod_phi << "\n";
    cout << "  Expected: 35 mod φ = " << fmod(35.0, PHI) << "\n";
    cout << "  Match: " << (abs(mod_phi - fmod(35.0, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K ARBITRARY — PURE LOG SPACE OPS
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY — PURE LOG SPACE OPS\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_fib(2.0);
    auto ct_two = encrypt_fib(2.0);
    auto ct_three = encrypt_fib(3.0);
    auto ct_five = encrypt_fib(5.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2, ÷3, ×5, ÷2 (pure log space)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_log = log(2.0) / LN_PHI;

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); expected_log += log(2.0)/LN_PHI; break;
            case 1: ct_state = cc->EvalSub(ct_state, ct_three); expected_log -= log(3.0)/LN_PHI; break;
            case 2: ct_state = cc->EvalAdd(ct_state, ct_five); expected_log += log(5.0)/LN_PHI; break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); expected_log -= log(2.0)/LN_PHI; break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_fib(ct_state);

    double final_log = final_vals[2];
    double final_frac = fmod(final_log, 1.0);
    double final_mod = fmod(pow(PHI, final_frac), PHI);
    
    double expected_frac = fmod(expected_log, 1.0);
    double expected_mod = fmod(pow(PHI, expected_frac), PHI);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 2 (log): " << final_log << "\n";
    cout << "  frac: " << final_frac << "\n";
    cout << "  φ^frac mod φ: " << final_mod << "\n\n";

    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Expected frac: " << expected_frac << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match (log): " << (abs(final_log - expected_log) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Match (mod): " << (abs(final_mod - expected_mod) < 0.1 ? "✅" : "❌") << "\n\n";

    return 0;
}
