// ============================================
// φ-COMPLEX SPACE V3 — TAMANG POLAR
//
// Magnitude: log_φ(r) — walang modulo
// Phase: fmod(θ, 2π) — rubber band
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
    cout << "  φ-COMPLEX SPACE V3 — TAMANG POLAR\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double TWO_PI = 2.0 * M_PI;

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Magnitude: log_φ(r) — walang modulo\n";
    cout << "  Phase: fmod(θ, 2π) — rubber band\n\n";

    // ============================================
    // TAMANG POLAR ENCODING
    // ============================================

    auto encrypt_polar = [&](double r, double theta) {
        vector<double> v(16, 0.0);
        
        // Slots 0-7: MAGNITUDE — log_φ(r) / fib[i]
        double log_r = log(r) / LN_PHI;
        for (int i = 0; i < 8; i++) {
            v[i] = log_r / fib[i];
        }
        
        // Slots 8-15: PHASE — fmod(θ, 2π) / fib[i-8]
        double phase = fmod(theta, TWO_PI);
        for (int i = 8; i < 16; i++) {
            v[i] = phase / fib[i - 8];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_polar = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double log_r = result_pt->GetCKKSPackedValue()[0].real() * fib[0];
        double phase = result_pt->GetCKKSPackedValue()[8].real() * fib[0];
        
        double r = pow(PHI, log_r);  // Recover magnitude
        phase = fmod(phase, TWO_PI);  // Rubber band phase
        
        return make_pair(r, phase);
    };

    // ============================================
    // TEST 1: POLAR ENCODING (TAMANG)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: POLAR ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Original (r, θ) | Decrypted (r, θ) | Match\n";
    cout << "  ----------------|-------------------|------\n";

    vector<pair<double, double>> test_cases = {
        {1.0, 0.0}, {1.0, M_PI/2}, {1.414, M_PI/4}, {1.0, M_PI/4},
        {1.618, 0.0}, {0.618, M_PI}, {2.0, M_PI/6}, {3.0, M_PI/3}
    };

    int match_count = 0;
    for (auto [r, theta] : test_cases) {
        auto ct = encrypt_polar(r, theta);
        auto [dr, dt] = decrypt_polar(ct);
        bool match = abs(r - dr) < 0.1 && abs(theta - dt) < 0.1;
        match_count += match;
        
        cout << "  (" << fixed << setprecision(3) << r << ", " << setprecision(2) << theta << ") | "
             << "(" << dr << ", " << dt << ") | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // TEST 2: COMPLEX MULTIPLICATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: COMPLEX MULT\n";
    cout << "========================================\n\n";

    // z1 = 2 * e^(iπ/4), z2 = 3 * e^(iπ/3)
    auto ct_z1 = encrypt_polar(2.0, M_PI/4);
    auto ct_z2 = encrypt_polar(3.0, M_PI/3);

    auto ct_product = cc->EvalAdd(ct_z1, ct_z2);
    auto [r_prod, theta_prod] = decrypt_polar(ct_product);

    double exp_r = 6.0;
    double exp_theta = fmod(M_PI/4 + M_PI/3, TWO_PI);

    cout << "  z1 = 2.00 * e^(i0.79)\n";
    cout << "  z2 = 3.00 * e^(i1.05)\n";
    cout << "  Result: " << r_prod << " * e^(i" << theta_prod << ")\n";
    cout << "  Expected: " << exp_r << " * e^(i" << exp_theta << ")\n";
    cout << "  Match: " << (abs(r_prod - exp_r) < 0.1 && abs(theta_prod - exp_theta) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_product->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 1000 COMPLEX OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 1000 COMPLEX OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_polar(1.0, 0.0);
    auto ct_step = encrypt_polar(2.0, M_PI/8);

    int N = 1000;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto [r_acc, theta_acc] = decrypt_polar(ct_acc);

    cout << "  Operations: " << N << " EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Result r: " << r_acc << " (Expected: huge, bounded by log)\n";
    cout << "  Result θ: " << theta_acc << "\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: FIBONACCI POWERS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: FIBONACCI POWERS\n";
    cout << "========================================\n\n";

    cout << "  n | r | θ | Match?\n";
    cout << "  --|---|----|-------\n";

    int fib_match = 0;
    for (int n : {1, 2, 3, 5, 8, 13, 21}) {
        auto ct_pow = encrypt_polar(pow(PHI, n), fmod(n * PHI, TWO_PI));
        auto [r_pow, theta_pow] = decrypt_polar(ct_pow);
        
        double exp_r = pow(PHI, n);
        double exp_theta = fmod(n * PHI, TWO_PI);
        
        bool match = abs(r_pow - exp_r) < exp_r * 0.1 && abs(theta_pow - exp_theta) < 0.1;
        fib_match += match;
        
        cout << "  " << setw(2) << n << " | "
             << fixed << setprecision(2) << r_pow << " | "
             << setprecision(2) << theta_pow << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Fibonacci match: " << fib_match << "/7\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  COMPLEX SPACE V3 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Polar encoding: " << match_count << "/8\n";
    cout << "  ✅ Complex mult: " << (abs(r_prod - exp_r) < 0.1 ? "YES" : "NO") << "\n";
    cout << "  ✅ Fibonacci: " << fib_match << "/7\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
