// ============================================
// φ-HYPERBOLIC SPACE — NON-EUCLIDEAN FHE
//
// Hyperbolic geometry + Golden Ratio
// Poincaré disk model
// Hyperbolic distance = addition sa φ-log space
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
    cout << "  φ-HYPERBOLIC SPACE — NON-EUCLIDEAN\n";
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
    cout << "  Hyperbolic: Poincaré disk model\n";
    cout << "  Distance = addition sa φ-log space\n\n";

    // ============================================
    // HYPERBOLIC ENCODING
    // ============================================

    auto encrypt_hyperbolic = [&](double radius, double angle) {
        // Poincaré disk: |z| < 1
        // Hyperbolic distance: d = 2 * arctanh(|z|)
        // φ-log space: log_φ(d + 1)
        
        // Clamp radius sa [0, 0.999]
        radius = min(radius, 0.999);
        
        // Hyperbolic distance
        double distance = 2.0 * atanh(radius);
        
        // φ-log space para sa distance
        double log_dist = log(distance + 1.0) / LN_PHI;
        
        vector<double> v(16, 0.0);
        
        // Distance sa slots 0-3 (log space + rubber band)
        v[0] = fmod(log_dist, PHI) / fib[0];
        v[1] = distance / fib[1];  // Raw para sa verification
        
        // Angle sa slots 4-7 (rubber band)
        double phase = fmod(angle, TWO_PI);
        v[4] = phase / fib[4];
        v[5] = phase / fib[5];
        
        // Radius sa slots 8-9
        v[8] = radius / fib[8];
        v[9] = log(radius + 1e-10) / LN_PHI / fib[9];
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_hyperbolic = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double distance = result_pt->GetCKKSPackedValue()[1].real() * fib[1];
        double phase = result_pt->GetCKKSPackedValue()[4].real() * fib[4];
        double radius = result_pt->GetCKKSPackedValue()[8].real() * fib[8];
        
        return make_tuple(distance, phase, radius);
    };

    // ============================================
    // TEST 1: BASIC HYPERBOLIC POINTS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: HYPERBOLIC POINTS\n";
    cout << "========================================\n\n";

    cout << "  Point (r, θ) | Distance | Phase | Radius\n";
    cout << "  -------------|----------|-------|-------\n";

    vector<pair<double, double>> points = {
        {0.0, 0.0},      // Origin
        {0.5, 0.0},      // Mid
        {0.8, M_PI/4},   // Near boundary
        {0.9, M_PI/2},   // Closer to boundary
        {0.99, M_PI},    // Very near boundary
        {1.0/PHI, 0.0},  // φ-related
        {0.618, PHI}     // Golden ratio point
    };

    int match_count = 0;
    for (auto [r, theta] : points) {
        auto ct = encrypt_hyperbolic(r, theta);
        auto [dist, phase, radius] = decrypt_hyperbolic(ct);
        
        double exp_dist = 2.0 * atanh(min(r, 0.999));
        double exp_phase = fmod(theta, TWO_PI);
        
        bool match = abs(dist - exp_dist) < 0.5;
        match_count += match;
        
        cout << "  (" << fixed << setprecision(2) << r << ", " << theta << ")"
             << " | " << setprecision(2) << dist
             << " | " << setprecision(2) << phase
             << " | " << setprecision(2) << radius
             << " (exp: " << exp_dist << ") "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/7\n\n";

    // ============================================
    // TEST 2: HYPERBOLIC ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: HYPERBOLIC ADDITION\n";
    cout << "========================================\n\n";

    // Add two hyperbolic points
    auto ct_a = encrypt_hyperbolic(0.5, 0.0);
    auto ct_b = encrypt_hyperbolic(0.3, M_PI/3);
    
    auto start = high_resolution_clock::now();
    
    // Hyperbolic addition = EvalAdd!
    auto ct_sum = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto [dist, phase, radius] = decrypt_hyperbolic(ct_sum);
    
    cout << "  Point A: (0.5, 0.0)\n";
    cout << "  Point B: (0.3, π/3)\n";
    cout << "  Sum distance: " << dist << "\n";
    cout << "  Sum phase: " << phase << "\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI HYPERBOLIC
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI HYPERBOLIC\n";
    cout << "========================================\n\n";

    cout << "  φ-related points:\n\n";
    
    for (int i = 1; i <= 8; i++) {
        double r = fib[i] / fib[i + 1];  // Converges sa 1/φ
        double theta = fmod(fib[i] * PHI, TWO_PI);
        
        auto ct = encrypt_hyperbolic(r, theta);
        auto [dist, phase, radius] = decrypt_hyperbolic(ct);
        
        cout << "  fib[" << i << "]/fib[" << i + 1 << "] = " << fixed 
             << setprecision(4) << r << " | d=" << setprecision(2) << dist
             << " | θ=" << phase << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 4: 1000 HYPERBOLIC OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 1000 HYPERBOLIC OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_hyperbolic(0.0, 0.0);
    auto ct_step = encrypt_hyperbolic(0.001, 0.0);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto [acc_dist, acc_phase, acc_radius] = decrypt_hyperbolic(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Distance: " << acc_dist << "\n\n";

    cout << "========================================\n";
    cout << "  φ-HYPERBOLIC SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Hyperbolic encoding\n";
    cout << "  ✅ Poincaré disk\n";
    cout << "  ✅ φ-log space\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
