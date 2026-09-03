// ============================================
// φ-RESTRUCTURE CORRECT
// TAMANG encoding: log_φ(x) walang modulo
// Modulo lang sa reconstruction
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
    cout << "  φ-RESTRUCTURE CORRECT\n";
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
    // TAMANG ENCODING — walang modulo sa log
    // Slot 0: F_{n-1}
    // Slot 1: F_n
    // Slot 2: log_φ(x) — WALANG MODULO
    // Slot 3: φ^frac
    // ============================================

    auto encrypt_ok = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;  // TAMA: walang modulo
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        
        vector<double> v(4, 0.0);
        v[0] = fib[n-1];
        v[1] = fib[n];
        v[2] = log_phi_x;
        v[3] = phi_frac;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_ok = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_ok(5.0);
    auto ct_7 = encrypt_ok(7.0);

    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_ok(ct_mult);

    double log_result = mult_vals[2];
    double result = pow(PHI, log_result);
    double mod_phi = fmod(result, PHI);

    cout << "  Slot 2 (log): " << log_result << "\n";
    cout << "  φ^(log) = " << result << "\n";
    cout << "  φ^(log) mod φ = " << mod_phi << "\n";
    cout << "  Expected: 35 mod φ = " << fmod(35.0, PHI) << "\n";
    cout << "  Match: " << (abs(mod_phi - fmod(35.0, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K ARBITRARY — TAMANG LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY — TAMANG LOG SPACE\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_ok(2.0);
    auto ct_two = encrypt_ok(2.0);
    auto ct_three = encrypt_ok(3.0);
    auto ct_five = encrypt_ok(5.0);
    auto ct_one = encrypt_ok(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2, ÷3, ×5, ÷2, ×1\n";
    cout << "  (Pure multiplication/division sa log space)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_log = log(2.0) / LN_PHI;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); expected_log += log(2.0)/LN_PHI; break;
            case 1: ct_state = cc->EvalSub(ct_state, ct_three); expected_log -= log(3.0)/LN_PHI; break;
            case 2: ct_state = cc->EvalAdd(ct_state, ct_five); expected_log += log(5.0)/LN_PHI; break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); expected_log -= log(2.0)/LN_PHI; break;
            case 4: ct_state = cc->EvalAdd(ct_state, ct_one); expected_log += log(1.0)/LN_PHI; break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_ok(ct_state);

    double result_log = final_vals[2];
    double result_mod = fmod(pow(PHI, result_log), PHI);
    double expected_mod = fmod(pow(PHI, expected_log), PHI);

    cout << "  ✅ 10K operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 2 (log): " << result_log << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  φ^(log) mod φ: " << result_mod << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match: " << (abs(result_log - expected_log) < 0.1 ? "✅" : "❌") << "\n\n";

    return 0;
}
