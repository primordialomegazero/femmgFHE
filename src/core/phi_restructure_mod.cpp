// ============================================
// φ-RESTRUCTURE MOD
// May φ-modulo para ma-bound ang log space
// Natural na periodicity
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
    cout << "  φ-RESTRUCTURE MOD\n";
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
    // ENCODING NA MAY MODULO
    // Slot 0: F_{n-1} mod φ
    // Slot 1: F_n mod φ
    // Slot 2: log_φ(x) mod φ
    // Slot 3: φ^frac mod φ
    // ============================================

    auto encrypt_mod = [&](double x) {
        double log_phi_x = fmod(log(x) / LN_PHI, PHI);
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = fmod(pow(PHI, frac), PHI);
        
        vector<double> v(4, 0.0);
        v[0] = fmod((double)fib[n-1], PHI);
        v[1] = fmod((double)fib[n], PHI);
        v[2] = log_phi_x;
        v[3] = phi_frac;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mod = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_mod(5.0);
    auto ct_7 = encrypt_mod(7.0);

    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_mod(ct_mult);

    double log_mod = fmod(mult_vals[2], PHI);
    double result = pow(PHI, log_mod);
    double mod_phi = fmod(result, PHI);

    cout << "  Slot 2 (log mod): " << log_mod << "\n";
    cout << "  φ^(log mod) = " << result << "\n";
    cout << "  φ^(log mod) mod φ = " << mod_phi << "\n";
    cout << "  Expected: 35 mod φ = " << fmod(35.0, PHI) << "\n";
    cout << "  Match: " << (abs(mod_phi - fmod(35.0, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K ARBITRARY WITH MOD
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY WITH MOD\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_mod(2.0);
    auto ct_two = encrypt_mod(2.0);
    auto ct_three = encrypt_mod(3.0);
    auto ct_five = encrypt_mod(5.0);
    auto ct_one = encrypt_mod(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×3, -1, ÷2, +5\n";
    cout << "  With φ-modulo\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); expected = fmod(expected + 2.0, PHI); break;
            case 1: ct_state = cc->EvalAdd(ct_state, ct_three); expected = fmod(expected * 3.0, PHI); break;
            case 2: ct_state = cc->EvalSub(ct_state, ct_one); expected = fmod(expected - 1.0, PHI); break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); expected = fmod(expected / 2.0, PHI); break;
            case 4: ct_state = cc->EvalAdd(ct_state, ct_five); expected = fmod(expected + 5.0, PHI); break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_mod(ct_state);

    double final_log_mod = fmod(final_vals[2], PHI);
    double final_result = pow(PHI, final_log_mod);
    double final_mod_phi = fmod(final_result, PHI);

    cout << "  ✅ 10K arbitrary complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n";
    cout << "  Slot 2 mod φ: " << final_log_mod << "\n";
    cout << "  φ^(log mod) mod φ: " << final_mod_phi << "\n\n";

    cout << "  Expected mod φ: " << expected << "\n";
    cout << "  Match: " << (abs(final_mod_phi - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    return 0;
}
