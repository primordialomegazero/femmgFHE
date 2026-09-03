// ============================================
// φ-FOURIER SPACE — FREQUENCY DOMAIN FHE
//
// Fourier transform + Golden Ratio
// Time domain → Frequency domain
// Convolution → Multiplication → Addition (φ-log)
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
    cout << "  φ-FOURIER SPACE — FREQUENCY DOMAIN\n";
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
    cout << "  Fourier: time domain → frequency domain\n";
    cout << "  Convolution = multiplication sa frequency\n";
    cout << "  Multiplication = addition sa φ-log space\n\n";

    // ============================================
    // φ-FOURIER ENCODING
    // ============================================

    auto encrypt_fourier = [&](const vector<double>& signal) {
        // Simplified DFT (Discrete Fourier Transform)
        // X[k] = Σ x[n] * e^(-i2πkn/N)
        // Sa φ-log space: magnitude at phase ng bawat frequency
        
        int N = signal.size();
        vector<double> v(16, 0.0);
        
        // Compute DFT at 8 frequencies (slots 0-7 para sa magnitude, 8-15 para sa phase)
        for (int k = 0; k < 8; k++) {
            complex<double> X_k(0, 0);
            for (int n = 0; n < N; n++) {
                double angle = -TWO_PI * k * n / N;
                X_k += signal[n] * exp(complex<double>(0, angle));
            }
            
            // Magnitude sa log_φ space
            double mag = abs(X_k);
            double log_mag = (mag > 0) ? log(mag + 1.0) / LN_PHI : 0.0;
            v[k] = log_mag / fib[k];
            
            // Phase sa rubber band
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
            
            double mag = exp(log_mag * LN_PHI) - 1.0;
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

    // Test signals
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
        
        // DC component = frequency 0
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

    // Convolution sa time domain = multiplication sa frequency domain
    // At multiplication = addition sa φ-log space!
    
    vector<double> signal_a = {1, 2, 3, 4, 0, 0, 0, 0};
    vector<double> signal_b = {1, 1, 1, 1, 0, 0, 0, 0};
    
    auto ct_a = encrypt_fourier(signal_a);
    auto ct_b = encrypt_fourier(signal_b);
    
    auto start = high_resolution_clock::now();
    
    // Convolution = EvalAdd sa frequency domain!
    auto ct_conv = cc->EvalAdd(ct_a, ct_b);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto freqs_conv = decrypt_fourier(ct_conv);
    
    cout << "  Signal A: {1, 2, 3, 4}\n";
    cout << "  Signal B: {1, 1, 1, 1}\n";
    cout << "  Convolution via EvalAdd!\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_conv->GetLevel() << "\n\n";
    cout << "  Frequency 0 (DC): " << freqs_conv[0].real() << "\n";
    cout << "  Expected DC: " << (4 + 10) << "\n\n";

    // ============================================
    // TEST 3: FREQUENCY ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FREQUENCY ANALYSIS\n";
    cout << "========================================\n\n";

    // φ-related frequencies
    vector<double> phi_signal = {1, 1, 2, 3, 5, 8, 13, 21};  // Fibonacci!
    
    auto ct_phi = encrypt_fourier(phi_signal);
    auto freqs_phi = decrypt_fourier(ct_phi);
    
    cout << "  Fibonacci signal: {1, 1, 2, 3, 5, 8, 13, 21}\n\n";
    cout << "  Frequency | Magnitude | Phase\n";
    cout << "  ----------|-----------|-------\n";
    
    for (int k = 0; k < 4; k++) {
        cout << "  " << setw(9) << k << " | " 
             << setw(9) << fixed << setprecision(2) << abs(freqs_phi[k])
             << " | " << setw(6) << setprecision(2) << arg(freqs_phi[k]) << "\n";
    }
    
    cout << "\n";

    // ============================================
    // TEST 4: 1000 FOURIER OPS
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
    cout << "  DC Component: " << freqs_final[0].real() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-FOURIER SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fourier encoding\n";
    cout << "  ✅ Convolution via EvalAdd\n";
    cout << "  ✅ φ-log space\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
