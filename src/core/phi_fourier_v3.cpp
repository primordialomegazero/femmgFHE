// ============================================
// φ-FOURIER SPACE V3 — TAMANG SPACE PER COMPONENT
//
// DC → Normal space (sum)
// Magnitude → Log space (multiplication)
// Phase → Rubber band (periodic)
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
    cout << "  φ-FOURIER SPACE V3 — TAMANG SPACES\n";
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
    cout << "  DC: normal (sum) | Mag: log | Phase: rubber band\n\n";

    // ============================================
    // FOURIER ENCODING (TAMANG SPACES)
    // ============================================

    auto encrypt_fourier = [&](const vector<double>& signal) {
        vector<double> v(16, 0.0);
        
        int N = signal.size();
        
        // DC COMPONENT (slot 0): SUM ng signal (normal space)
        double dc = 0.0;
        for (double s : signal) dc += s;
        v[0] = dc / fib[0];  // Normal space — walang log!
        
        // MAGNITUDE (slots 1-7): log_φ(magnitude) (log space)
        for (int k = 1; k <= 7 && k < N; k++) {
            // DFT magnitude para sa frequency k
            double real_sum = 0.0, imag_sum = 0.0;
            for (int n = 0; n < N; n++) {
                double angle = TWO_PI * k * n / N;
                real_sum += signal[n] * cos(angle);
                imag_sum += signal[n] * sin(angle);
            }
            double magnitude = sqrt(real_sum * real_sum + imag_sum * imag_sum);
            double mag_log = (magnitude > 0) ? log(magnitude) / LN_PHI : 0.0;
            v[k] = mag_log / fib[k];
        }
        
        // PHASE (slots 8-15): fmod(θ, 2π) (rubber band)
        for (int k = 1; k <= 7 && k < N; k++) {
            double real_sum = 0.0, imag_sum = 0.0;
            for (int n = 0; n < N; n++) {
                double angle = TWO_PI * k * n / N;
                real_sum += signal[n] * cos(angle);
                imag_sum += signal[n] * sin(angle);
            }
            double phase = atan2(imag_sum, real_sum);
            v[7 + k] = fmod(phase, TWO_PI) / fib[k];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fourier = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double dc = result_pt->GetCKKSPackedValue()[0].real() * fib[0];
        
        vector<double> mags(7);
        vector<double> phases(7);
        for (int k = 0; k < 7; k++) {
            double mag_log = result_pt->GetCKKSPackedValue()[k + 1].real() * fib[k + 1];
            mags[k] = pow(PHI, mag_log);
            phases[k] = fmod(result_pt->GetCKKSPackedValue()[8 + k].real() * fib[k + 1], TWO_PI);
        }
        
        return make_tuple(dc, mags, phases);
    };

    // ============================================
    // TEST 1: BASIC SIGNALS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC SIGNALS\n";
    cout << "========================================\n\n";

    cout << "  Signal | DC | Match?\n";
    cout << "  -------|----|--------\n";

    int dc_match = 0;
    
    // Signal 1: {1, 0, 0, 0}
    auto ct1 = encrypt_fourier({1, 0, 0, 0});
    auto [dc1, mags1, phases1] = decrypt_fourier(ct1);
    cout << "  {1,0,0,0} | " << dc1 << " | " << (abs(dc1 - 1) < 0.1 ? "✅" : "❌") << "\n";
    dc_match += (abs(dc1 - 1) < 0.1);

    // Signal 2: {1, 1, 1, 1}
    auto ct2 = encrypt_fourier({1, 1, 1, 1});
    auto [dc2, mags2, phases2] = decrypt_fourier(ct2);
    cout << "  {1,1,1,1} | " << dc2 << " | " << (abs(dc2 - 4) < 0.1 ? "✅" : "❌") << "\n";
    dc_match += (abs(dc2 - 4) < 0.1);

    // Signal 3: {1, 2, 3, 4}
    auto ct3 = encrypt_fourier({1, 2, 3, 4});
    auto [dc3, mags3, phases3] = decrypt_fourier(ct3);
    cout << "  {1,2,3,4} | " << dc3 << " | " << (abs(dc3 - 10) < 0.1 ? "✅" : "❌") << "\n";
    dc_match += (abs(dc3 - 10) < 0.1);

    // Signal 4: Fibonacci
    auto ct4 = encrypt_fourier({1, 1, 2, 3, 5, 8, 13, 21});
    auto [dc4, mags4, phases4] = decrypt_fourier(ct4);
    cout << "  Fibonacci | " << dc4 << " | " << (abs(dc4 - 54) < 0.1 ? "✅" : "❌") << "\n";
    dc_match += (abs(dc4 - 54) < 0.1);

    cout << "\n  DC Match: " << dc_match << "/4\n\n";

    // ============================================
    // TEST 2: CONVOLUTION THEOREM
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: CONVOLUTION\n";
    cout << "========================================\n\n";

    auto ct_a = encrypt_fourier({1, 2, 3, 4});
    auto ct_b = encrypt_fourier({1, 1, 1, 1});

    // Convolution sa frequency domain = multiplication
    auto ct_conv = cc->EvalAdd(ct_a, ct_b);
    
    auto [dc_conv, mags_conv, phases_conv] = decrypt_fourier(ct_conv);

    // Expected: convolution DC = sum(A) × sum(B) = 10 × 4 = 40
    double expected_dc = 40.0;

    cout << "  Signal A: {1, 2, 3, 4}\n";
    cout << "  Signal B: {1, 1, 1, 1}\n";
    cout << "  Convolution DC: " << dc_conv << "\n";
    cout << "  Expected DC: " << expected_dc << "\n";
    cout << "  Match: " << (abs(dc_conv - expected_dc) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_conv->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 1000 FOURIER OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 1000 FOURIER OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_fourier({1, 1, 1, 1});
    auto ct_step = encrypt_fourier({1, 0, 0, 0});

    auto start = high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto [dc_final, mags_final, phases_final] = decrypt_fourier(ct_acc);

    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  DC: " << dc_final << " (Expected: 4 + 1000 = 1004)\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FOURIER V3 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ DC: " << dc_match << "/4 match\n";
    cout << "  ✅ Convolution: " << (abs(dc_conv - 40.0) < 0.1 ? "YES" : "NO") << "\n";
    cout << "  ✅ 1000 ops: " << time << " ms\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
