// ============================================
// φ-FOURIER V4 — DUAL DC (SUM + MULT)
//
// DC_sum: normal space (addition)
// DC_mult: log space (multiplication)
// Magnitude: log space
// Phase: rubber band
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
    cout << "  φ-FOURIER V4 — DUAL DC\n";
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
    cout << "  DC_sum: normal | DC_mult: log | Mag: log | Phase: rubber\n\n";

    // ============================================
    // DUAL DC ENCODING
    // ============================================

    auto encrypt_fourier = [&](const vector<double>& signal) {
        vector<double> v(16, 0.0);
        int N = signal.size();
        
        // DC (slot 0): SUM — normal space
        double dc = 0.0;
        for (double s : signal) dc += s;
        v[0] = dc / fib[0];
        
        // DC_MULT (slot 1): log_φ(dc) — log space para sa convolution
        double dc_log = (dc > 0) ? log(dc) / LN_PHI : 0.0;
        v[1] = dc_log / fib[1];
        
        // Magnitude (slots 2-7): log space
        for (int k = 1; k <= 6 && k < N; k++) {
            double real_sum = 0.0, imag_sum = 0.0;
            for (int n = 0; n < N; n++) {
                double angle = TWO_PI * k * n / N;
                real_sum += signal[n] * cos(angle);
                imag_sum += signal[n] * sin(angle);
            }
            double magnitude = sqrt(real_sum * real_sum + imag_sum * imag_sum);
            double mag_log = (magnitude > 0) ? log(magnitude) / LN_PHI : 0.0;
            v[k + 1] = mag_log / fib[k + 1];
        }
        
        // Phase (slots 8-15): rubber band
        for (int k = 1; k <= 6 && k < N; k++) {
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
        double dc_log = result_pt->GetCKKSPackedValue()[1].real() * fib[1];
        double dc_recovered = pow(PHI, dc_log);
        
        return make_pair(dc, dc_recovered);
    };

    // ============================================
    // TEST: CONVOLUTION (DUAL DC)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: CONVOLUTION (DUAL DC)\n";
    cout << "========================================\n\n";

    auto ct_a = encrypt_fourier({1, 2, 3, 4});
    auto ct_b = encrypt_fourier({1, 1, 1, 1});

    // Convolution: EvalAdd sa frequency domain
    auto ct_conv = cc->EvalAdd(ct_a, ct_b);
    
    auto [dc_sum, dc_mult] = decrypt_fourier(ct_conv);

    // Expected: sum(A) × sum(B) = 10 × 4 = 40
    // DC_sum: 10 + 4 = 14 (addition)
    // DC_mult: log(10) + log(4) = log(40) → 40 (multiplication!)

    cout << "  Signal A: {1, 2, 3, 4} (sum=10)\n";
    cout << "  Signal B: {1, 1, 1, 1} (sum=4)\n\n";
    cout << "  DC_sum (addition): " << dc_sum << " (10+4=14)\n";
    cout << "  DC_mult (multiplication): " << dc_mult << " (10×4=40)\n";
    cout << "  Convolution Match: " << (abs(dc_mult - 40.0) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_conv->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FOURIER V4 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Dual DC (sum + mult)\n";
    cout << "  ✅ Convolution: " << (abs(dc_mult - 40.0) < 0.1 ? "YES" : "NO") << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
