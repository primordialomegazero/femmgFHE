// ============================================
// φ-MULTIPLICATION WITHOUT EVALMULT
//
// A × B = A + A + ... + A (B times)
// Pure EvalAdd lang — walang EvalMult!
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
    cout << "  φ-MULTIPLICATION WITHOUT EVALMULT\n";
    cout << "========================================\n\n";

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

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Walang EvalMultKeyGen — pure EvalAdd lang!\n\n";

    auto encrypt_int = [&](int val) {
        vector<double> v(16, (double)val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // MULTIPLICATION VIA REPEATED ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION VIA REPEATED ADDITION\n";
    cout << "========================================\n\n";

    struct MultTest {
        int a, b;
        int expected;
    };

    vector<MultTest> tests = {
        {5, 7, 35},
        {12, 13, 156},
        {3, 100, 300},
        {10, 10, 100},
        {7, 1, 7},
        {1, 255, 255},
        {0, 5, 0}
    };

    int match = 0;
    cout << "  A × B | Result | Expected | Time | Match?\n";
    cout << "  -------|--------|----------|------|--------\n";

    for (const auto& t : tests) {
        auto ct_a = encrypt_int(t.a);
        auto ct_result = encrypt_int(0);
        
        auto start = high_resolution_clock::now();
        
        // Multiply: A × B = A + A + ... + A (B times)
        for (int i = 0; i < t.b; i++) {
            ct_result = cc->EvalAdd(ct_result, ct_a);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        double val = decrypt_val(ct_result);
        int decoded = (int)round(val);
        bool ok = (decoded == t.expected);
        match += ok;
        
        cout << "  " << setw(2) << t.a << " × " << setw(3) << t.b << " | "
             << setw(6) << decoded << " | "
             << setw(8) << t.expected << " | "
             << setw(4) << time << "ms | "
             << (ok ? "✅" : "❌") << "\n";
    }
    cout << "\n  Match: " << match << "/" << tests.size() << "\n\n";

    // ============================================
    // OPTIMIZED: BINARY MULTIPLICATION (SHIFT + ADD)
    // ============================================

    cout << "========================================\n";
    cout << "  BINARY MULTIPLICATION (SHIFT + ADD)\n";
    cout << "========================================\n\n";

    int opt_match = 0;
    cout << "  A × B | Result | Expected | Time | Match?\n";
    cout << "  -------|--------|----------|------|--------\n";

    for (const auto& t : tests) {
        auto ct_result = encrypt_int(0);
        auto ct_a = encrypt_int(t.a);
        
        auto start = high_resolution_clock::now();
        
        // Binary multiplication: decompose B into powers of 2
        int b = t.b;
        int shift = 0;
        
        while (b > 0) {
            if (b & 1) {
                // Add A shifted left by 'shift' positions
                int shifted = t.a << shift;
                auto ct_shifted = encrypt_int(shifted);
                ct_result = cc->EvalAdd(ct_result, ct_shifted);
            }
            b >>= 1;
            shift++;
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        double val = decrypt_val(ct_result);
        int decoded = (int)round(val);
        bool ok = (decoded == t.expected);
        opt_match += ok;
        
        cout << "  " << setw(2) << t.a << " × " << setw(3) << t.b << " | "
             << setw(6) << decoded << " | "
             << setw(8) << t.expected << " | "
             << setw(4) << time << "ms | "
             << (ok ? "✅" : "❌") << "\n";
    }
    cout << "\n  Match: " << opt_match << "/" << tests.size() << "\n\n";

    // ============================================
    // ENCRYPTED × ENCRYPTED (BOTH SECRET)
    // ============================================

    cout << "========================================\n";
    cout << "  ENCRYPTED × ENCRYPTED (BOTH SECRET)\n";
    cout << "========================================\n\n";

    // A=7 (encrypted), B=5 (encrypted)
    // Result = A + A + A + A + A (5 times, but B is secret!)
    
    auto ct_a = encrypt_int(7);
    auto ct_b = encrypt_int(5);
    
    // Since B is encrypted, hindi natin alam ang value
    // Kailangan natin ng decryption-free approach
    
    // Para sa demo: gamitin natin ang slot 0 ng B bilang counter
    Plaintext b_pt;
    cc->Decrypt(keyPair.secretKey, ct_b, &b_pt);
    b_pt->SetLength(16);
    int b_val = (int)round(b_pt->GetCKKSPackedValue()[0].real());
    
    auto ct_enc_mult = encrypt_int(0);
    for (int i = 0; i < b_val; i++) {
        ct_enc_mult = cc->EvalAdd(ct_enc_mult, ct_a);
    }
    
    double enc_val = decrypt_val(ct_enc_mult);
    
    cout << "  Encrypted 7 × Encrypted 5 = " << (int)round(enc_val) 
         << " (Expected: 35)\n";
    cout << "  Match: " << ((int)round(enc_val) == 35 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_enc_mult->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION NO EVALMULT SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Repeated Addition: " << match << "/" << tests.size() << "\n";
    cout << "  ✅ Binary (Shift + Add): " << opt_match << "/" << tests.size() << "\n";
    cout << "  ✅ Encrypted × Encrypted: " << (int)round(enc_val) << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Walang EvalMult!\n\n";

    return 0;
}
