// ============================================
// φ-DIAGNOSTIC: BAKIT LAGING KALAHATI?
//
// Test 1: Pure CKKS subtraction (walang φ-encoding)
// Test 2: φ-math plaintext (walang CKKS)
// Test 3: Combined (current setup mo)
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

const double PHI = 1.6180339887498948482;
const double HALF_PHI = PHI / 2.0;

int main() {
    cout << "========================================\n";
    cout << "  φ-DIAGNOSTIC: KALAHATI PROBLEM\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: PURE CKKS (No φ-encoding)
    // ============================================
    cout << "TEST 1: PURE CKKS SUBTRACTION\n";
    cout << "----------------------------------------\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    // Simple 64-value subtraction test
    vector<double> plain_a(16, 0.0), plain_b(16, 0.0);
    for (int i = 0; i < 16; i++) {
        plain_a[i] = (i % 2 == 0) ? 2.0 : -2.0;
        plain_b[i] = (i % 2 == 0) ? 1.0 : -1.0;
    }

    Plaintext pt_a = cc->MakeCKKSPackedPlaintext(plain_a);
    Plaintext pt_b = cc->MakeCKKSPackedPlaintext(plain_b);
    
    auto ct_a = cc->Encrypt(keyPair.publicKey, pt_a);
    auto ct_b = cc->Encrypt(keyPair.publicKey, pt_b);
    
    auto ct_result = cc->EvalSub(ct_a, ct_b);
    
    Plaintext pt_result;
    cc->Decrypt(keyPair.secretKey, ct_result, &pt_result);
    pt_result->SetLength(16);

    int match_ckks = 0;
    for (int i = 0; i < 16; i++) {
        double expected = plain_a[i] - plain_b[i];
        double actual = pt_result->GetCKKSPackedValue()[i].real();
        if (abs(expected - actual) < 0.1) match_ckks++;
    }
    
    cout << "  CKKS subtraction match: " << match_ckks << "/16\n";
    cout << "  Result: " << (match_ckks == 16 ? "✅ CKKS OK!" : "❌ CKKS problem!") << "\n\n";

    // ============================================
    // TEST 2: φ-MATH PLAINTEXT (No encryption)
    // ============================================
    cout << "TEST 2: φ-MATH PLAINTEXT\n";
    cout << "----------------------------------------\n";

    auto phi_encode = [](int bit) {
        return (bit == 0) ? -2.0 : 2.0;
    };

    auto phi_subtract = [&](double a, double b) {
        // Your φ-subtraction logic
        return a - b;
    };

    int match_phi = 0;
    for (int bit_a = 0; bit_a <= 1; bit_a++) {
        for (int bit_b = 0; bit_b <= 1; bit_b++) {
            double a = phi_encode(bit_a);
            double b = phi_encode(bit_b);
            double result = phi_subtract(a, b);
            
            // Decode logic (simplified)
            int expected = (bit_a - bit_b + 2) % 2;
            int actual = (result > 0) ? 1 : 0;
            
            if (expected == actual) match_phi++;
        }
    }
    
    cout << "  φ-math subtraction match: " << match_phi << "/4\n";
    cout << "  Result: " << (match_phi == 4 ? "✅ φ-math OK!" : "❌ φ-math problem!") << "\n\n";

    // ============================================
    // TEST 3: 64-BIT SIMULATION
    // ============================================
    cout << "TEST 3: 64-BIT SUBTRACTION SIMULATION\n";
    cout << "----------------------------------------\n";

    // Simulate 64-bit subtraction pattern
    vector<int> A64(64), B64(64);
    for (int i = 0; i < 64; i++) {
        A64[i] = (i % 4 == 0 || i % 4 == 1) ? 1 : 0;
        B64[i] = (i % 4 == 0 || i % 4 == 2) ? 1 : 0;
    }

    // Sequential subtraction simulation
    vector<int> borrow(65, 0);
    vector<int> diff64(64, 0);
    
    for (int i = 0; i < 64; i++) {
        int a = A64[i];
        int b = B64[i] + borrow[i];
        
        if (a >= b) {
            diff64[i] = a - b;
            borrow[i+1] = 0;
        } else {
            diff64[i] = a + 2 - b;
            borrow[i+1] = 1;
        }
    }

    // Check pattern
    int first_half_ok = 0, second_half_ok = 0;
    for (int i = 0; i < 32; i++) {
        if (diff64[i] == ((A64[i] - B64[i] + 2) % 2)) first_half_ok++;
    }
    for (int i = 32; i < 64; i++) {
        if (diff64[i] == ((A64[i] - B64[i] + 2) % 2)) second_half_ok++;
    }

    cout << "  First half (bits 0-31): " << first_half_ok << "/32\n";
    cout << "  Second half (bits 32-63): " << second_half_ok << "/32\n";
    cout << "  Pattern: " << (first_half_ok == 32 && second_half_ok < 32 ? 
         "KALAHATI PROBLEM DETECTED!" : "No kalahati pattern") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    cout << "========================================\n";
    cout << "  DIAGNOSTIC SUMMARY\n";
    cout << "========================================\n";
    cout << "  CKKS: " << (match_ckks == 16 ? "✅ OK" : "❌ FAIL") << "\n";
    cout << "  φ-math: " << (match_phi == 4 ? "✅ OK" : "❌ FAIL") << "\n";
    cout << "  Kalahati: " << (first_half_ok == 32 && second_half_ok < 32 ? 
         "🔥 DETECTED!" : "Not detected") << "\n\n";

    if (match_ckks == 16 && match_phi == 4) {
        cout << "  ✅ CKKS at φ-math parehong OK!\n";
        cout << "  ❌ Problem nasa sequential chain logic!\n";
    } else if (match_ckks < 16) {
        cout << "  ❌ CKKS ang may drift problem!\n";
    } else {
        cout << "  ❌ φ-math ang may encoding issue!\n";
    }

    return 0;
}
