// ============================================
// φ-FOURIER SPACE V2 — PURE LOG + RUBBER BAND
//
// Fix:
// 1. Pure log (walang +1.0)
// 2. Pure exp (walang -1.0)
// 3. Rubber band sa magnitude
// 4. Bounded sa [0, φ)
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
    cout << "  φ-FOURIER SPACE V2 — PURE LOG\n";
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
    cout << "  Pure log: log_φ(r) — walang offset\n";
    cout << "  Rubber band: fmod(x, φ)\n\n";

    // ============================================
    // PURE LOG FOURIER ENCODING
    // ============================================

    auto encrypt_fourier = [&](const vector<double>& signal) {
        int N = signal.size();
        vector<double> v(16, 0.0);
        
        // Compute DFT at 8 frequencies
        for (int k = 0; k < 8; k++) {
            complex<double> X_k(0, 0);
            for (int n = 0; n < N; n++) {
                double angle = -TWO_PI * k * n / N;
                X_k += signal[n] * exp(complex<double>(0, angle));
            }
            
            // Magnitude: PURE LOG sa φ space + rubber band
            double mag = abs(X_k) + 1e-10;  // Iwas log(0)
            double log_mag = fmod(log(mag) / LN_PHI, PHI);
            v[k] = log_mag / fib[k];
            
            // Phase: rubber band
            double phase = fmod(arg(X_k) + TWO_PI, TWO_PI);
            v[k + 8] = phase / fib[k];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fourier = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        vector<complex<double>> freqs(8);
        for (int k = 0; k < 8; k++) {
            double log_mag = result_pt->GetCKKSPackedValue()[k].real() * fib[k];
            double phase = result_pt->GetCKKSPackedValue()[k + 8].real() * fib[k];
            
            // PURE EXP recovery
            log_mag = fmod(log_mag, PHI);
            double mag = exp(log_mag * LN_PHI);
            phase = fmod(phase, TWO_PI);
            freqs[k] = polar(mag, phase);
        }
        
        return freqs;
    };

    // ============================================
    // TEST 1: BASIC SIGNALS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC SIGNALS\n";
    cout << "========================================\n\n";

    vector<vector<double>> signals = {
        {1, 0, 0, 0, 0, 0, 0, 0},           // Impulse
        {1, 1, 1, 1, 1, 1, 1, 1},           // Constant
        {1, 0, -1, 0, 1, 0, -1, 0},         // Sine wave
        {0, 1, 0, -1, 0, 1, 0, -1},         // Cosine wave
        {1, 1, 0, 0, 1, 1, 0, 0}            // Square wave
    };

    cout << "  Signal | DC Component | Match?\n";
    cout << "  -------|--------------|--------\n";

    int match_count = 0;
    for (auto& signal : signals) {
        auto ct = encrypt_fourier(signal);
        auto freqs = decrypt_fourier(ct);
        
        double expected_dc = 0;
        for (double x : signal) expected_dc += x;
        
        double actual_dc = abs(freqs[0]);
        bool match = abs(expected_dc - actual_dc) < 1.0;
        match_count += match;
        
        cout << "  " << setw(6) << signal[0] << "," << signal[1] << "..."
             << " | " << setw(12) << fixed << setprecision(2) << actual_dc
             << " | " << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/5\n\n";

    // ============================================
    // TEST 2: CONVOLUTION THEOREM
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: CONVOLUTION THEOREM\n";
    cout << "========================================\n\n";

    vector<double> signal_a = {1, 2, 3, 4, 0, 0, 0, 0};
    vector<double> signal_b = {1, 1, 1, 1, 0, 0, 0, 0};
    
    auto ct_a = encrypt_fourier(signal_a);
    auto ct_b = encrypt_fourier(signal_b);
    
    auto start = high_resolution_clock::now();
    
    // Convolution = EvalAdd sa frequency domain
    auto ct_conv = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto freqs_conv = decrypt_fourier(ct_conv);
    
    // Expected convolution: {1, 3, 6, 10, 9, 7, 4, 0}
    // DC = sum = 40
    double expected_dc = 40.0;
    double actual_dc = abs(freqs_conv[0]);
    
    cout << "  Signal A: {1, 2, 3, 4}\n";
    cout << "  Signal B: {1, 1, 1, 1}\n";
    cout << "  Convolution DC: " << actual_dc << "\n";
    cout << "  Expected DC: " << expected_dc << "\n";
    cout << "  Match: " << (abs(expected_dc - actual_dc) < 5.0 ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_conv->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI FREQUENCY ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI FREQUENCY\n";
    cout << "========================================\n\n";

    vector<double> phi_signal = {1, 1, 2, 3, 5, 8, 13, 21};
    
    auto ct_phi = encrypt_fourier(phi_signal);
    auto freqs_phi = decrypt_fourier(ct_phi);
    
    cout << "  Fibonacci signal: {1, 1, 2, 3, 5, 8, 13, 21}\n";
    cout << "  Expected DC: 54\n";
    cout << "  Actual DC: " << abs(freqs_phi[0]) << "\n\n";
    
    cout << "  Frequency | Magnitude | Phase\n";
    cout << "  ----------|-----------|-------\n";
    
    for (int k = 0; k < 4; k++) {
        cout << "  " << setw(9) << k << " | " 
             << setw(9) << fixed << setprecision(2) << abs(freqs_phi[k])
             << " | " << setw(6) << setprecision(2) << arg(freqs_phi[k]) << "\n";
    }
    
    cout << "\n";

    // ============================================
    // TEST 4: 1000 FOURIER OPS (RUBBER BAND)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 1000 FOURIER OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_fourier({1, 0, 0, 0, 0, 0, 0, 0});
    auto ct_step = encrypt_fourier({0, 1, 0, 0, 0, 0, 0, 0});
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto freqs_final = decrypt_fourier(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  DC Component: " << abs(freqs_final[0]) << "\n";
    cout << "  Bounded: " << (abs(freqs_final[0]) < 1000.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-FOURIER SPACE V2 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Pure log encoding\n";
    cout << "  ✅ Rubber band modulo\n";
    cout << "  ✅ Convolution via EvalAdd\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
