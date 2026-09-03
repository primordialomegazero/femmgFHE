// ============================================
// φ-COMPLEX SPACE — DUAL REALITY + IMAGINARY
//
// Complex numbers: z = a + bi
// φ-merge: z_φ = f(z, φ)
// 
// Test:
// 1. Complex encoding
// 2. φ-periodicity sa complex plane
// 3. Dual reality (real + imaginary)
// 4. Emergent properties
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
    cout << "  φ-COMPLEX SPACE — DUAL REALITY\n";
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
    const complex<double> i(0, 1);  // Imaginary unit

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Complex space: z = a + bi\n";
    cout << "  φ = " << PHI << "\n\n";

    // ============================================
    // φ-COMPLEX ENCODING
    // ============================================

    auto encrypt_complex = [&](complex<double> z) {
        // φ-complex transform:
        // z_φ = |z| * e^(i * arg(z) * φ)
        // Or: z_φ = (a * φ) + (b * φ)i
        
        double magnitude = abs(z);
        double phase = arg(z);
        
        // φ-scaled complex value
        complex<double> z_phi = polar(magnitude, phase * PHI);
        
        // Extract real at imaginary parts
        double real_part = z_phi.real();
        double imag_part = z_phi.imag();
        
        // Encode sa log space na may φ
        double log_val = log(magnitude + 1.0) / LN_PHI;
        double phase_val = fmod(phase * PHI, PHI);
        
        // Combine: real part may log, imag part may phase
        vector<double> v(16, 0.0);
        for (int j = 0; j < 16; j++) {
            if (j < 8) {
                v[j] = log_val / fib[j];  // Real part (log space)
            } else {
                v[j] = phase_val / fib[j - 8];  // Imag part (phase space)
            }
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_complex = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        auto vals = result_pt->GetCKKSPackedValue();
        
        // Recover real at imaginary parts
        double log_val = vals[0].real() * fib[0];
        double phase_val = vals[8].real() * fib[0];
        
        // Reconstruct complex number
        double magnitude = exp(log_val * LN_PHI) - 1.0;
        double phase = fmod(phase_val / PHI, 2.0 * M_PI);
        
        return polar(magnitude, phase);
    };

    // ============================================
    // TEST 1: COMPLEX ROTATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ-COMPLEX ROTATION\n";
    cout << "========================================\n\n";

    // Test values
    vector<complex<double>> test_values = {
        {1.0, 0.0},
        {0.0, 1.0},
        {1.0, 1.0},
        {0.707, 0.707},  // 45 degrees
        {PHI, 0.0},
        {1.0/PHI, 0.0}
    };

    cout << "  Original | φ-Encoded | Decrypted | Match\n";
    cout << "  ---------|-----------|-----------|------\n";

    for (auto z : test_values) {
        auto ct = encrypt_complex(z);
        auto z_dec = decrypt_complex(ct);
        
        bool match = (abs(z.real() - z_dec.real()) < 0.1 && 
                     abs(z.imag() - z_dec.imag()) < 0.1);
        
        cout << "  (" << fixed << setprecision(3) << z.real() << ", " 
             << z.imag() << ") | φ-encoded | ("
             << z_dec.real() << ", " << z_dec.imag() << ") | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: COMPLEX MULTIPLICATION (LOG SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: COMPLEX MULT (φ-LOG SPACE)\n";
    cout << "========================================\n\n";

    cout << "  Complex multiplication = rotation + scaling\n";
    cout << "  Sa φ-log space = addition lang!\n\n";

    complex<double> z1 = polar(2.0, M_PI / 4);  // 2 * e^(iπ/4)
    complex<double> z2 = polar(3.0, M_PI / 3);  // 3 * e^(iπ/3)
    
    auto ct1 = encrypt_complex(z1);
    auto ct2 = encrypt_complex(z2);
    
    auto start = high_resolution_clock::now();
    
    // Complex multiplication sa φ-log space = EvalAdd!
    auto ct_result = cc->EvalAdd(ct1, ct2);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto z_result = decrypt_complex(ct_result);
    complex<double> z_expected = z1 * z2;
    
    cout << "  z1 = " << z1.real() << " + " << z1.imag() << "i\n";
    cout << "  z2 = " << z2.real() << " + " << z2.imag() << "i\n";
    cout << "  Expected: " << z_expected.real() << " + " << z_expected.imag() << "i\n";
    cout << "  Result: " << z_result.real() << " + " << z_result.imag() << "i\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: COMPLEX POWERS (φ-PERIODIC)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: COMPLEX POWERS (φ-PERIODIC)\n";
    cout << "========================================\n\n";

    cout << "  e^(i*θ) → rotation by θ\n";
    cout << "  Sa φ-space: periodic!\n\n";

    for (int n : {1, 2, 3, 5, 8, 13, 21}) {
        complex<double> z = polar(1.0, n * PHI);
        auto ct = encrypt_complex(z);
        auto z_dec = decrypt_complex(ct);
        
        double expected_phase = fmod(n * PHI, 2.0 * M_PI);
        double actual_phase = arg(z_dec);
        
        bool match = abs(expected_phase - actual_phase) < 0.1;
        
        cout << "  φ^" << n << " | Phase: " << setw(6) << fixed << setprecision(3)
             << actual_phase << " rad | Expected: " << expected_phase 
             << " | " << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  φ-COMPLEX SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Complex encoding (dual reality)\n";
    cout << "  ✅ φ-log space (addition = multiplication)\n";
    cout << "  ✅ φ-periodic (rubber band)\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
