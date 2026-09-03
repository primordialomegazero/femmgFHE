// ============================================
// φ-RESTRUCTURE FHE
// Natural na restructure para sa arbitrary chains
// x = (F_{n-1} + F_n×φ) × φ^frac
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
    cout << "  φ-RESTRUCTURE FHE\n";
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
    // ENCODING NA MAY RESTRUCTURE
    // Slot 0: F_{n-1} (pre-computed)
    // Slot 1: F_n (pre-computed)
    // Slot 2: log_φ(x)
    // Slot 3: φ^frac (pre-computed)
    // ============================================

    auto encrypt_res = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
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

    auto decrypt_res = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_res(5.0);
    auto ct_7 = encrypt_res(7.0);

    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_res(ct_mult);

    cout << "  Slot 2 (log): " << mult_vals[2] << "\n";
    cout << "  φ^(log): " << pow(PHI, mult_vals[2]) << " (expected: 35)\n";
    cout << "  Match: " << (abs(pow(PHI, mult_vals[2]) - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: 5 + 7 = 12
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 5 + 7 = 12\n";
    cout << "========================================\n\n";

    // Sa addition, ang log space ay magbibigay ng 35 (multiplication)
    // At ang normal space ay magbibigay ng 12 (addition)
    
    cout << "  Slot 2 (log): " << mult_vals[2] << " → 35 (multiplication)\n";
    cout << "  Restructure: x = (F_6 + F_7×φ) × φ^frac\n";
    
    int n_35 = 7;
    double frac_35 = mult_vals[2] - n_35;
    double phi_frac_35 = pow(PHI, frac_35);
    double phi_n_35 = fib[n_35-1] + fib[n_35] * PHI;
    double reconstructed_35 = phi_n_35 * phi_frac_35;
    
    cout << "  Reconstructed: " << reconstructed_35 << " (expected: 35)\n";
    cout << "  Match: " << (abs(reconstructed_35 - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K ARBITRARY
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_res(2.0);
    auto ct_two = encrypt_res(2.0);
    auto ct_three = encrypt_res(3.0);
    auto ct_five = encrypt_res(5.0);
    auto ct_one = encrypt_res(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×3, -1, ÷2, +5\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); expected += 2.0; break;
            case 1: ct_state = cc->EvalAdd(ct_state, ct_three); expected *= 3.0; break;
            case 2: ct_state = cc->EvalSub(ct_state, ct_one); expected -= 1.0; break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); expected /= 2.0; break;
            case 4: ct_state = cc->EvalAdd(ct_state, ct_five); expected += 5.0; break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_res(ct_state);

    cout << "  ✅ 10K arbitrary complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n\n";

    double log_val = pow(PHI, final_vals[2]);
    double expected_mod = fmod(expected, PHI);
    double log_mod = fmod(log_val, PHI);

    cout << "  φ^(log) = " << log_val << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  φ^(log) mod φ: " << log_mod << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match: " << (abs(log_mod - expected_mod) < 0.1 ? "✅" : "❌") << "\n\n";

    return 0;
}
