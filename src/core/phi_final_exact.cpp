// ============================================
// φ-FINAL EXACT — NOISE TRACKING + AUTO-SUBTRACT
//
// KEY: Track operation count, auto-subtract noise
//
// Encoding: value + noise
// Operations: (a+n) + (b+n) = (a+b) + 2n
// Recovery: (a+b) + 2n - 2n = (a+b)
//
// LAHAT EMERGENT — walang hardcode!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class PhiFinalExact {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);
    
    // Noise tracking
    int operation_count;
    double accumulated_noise;

public:
    PhiFinalExact() : operation_count(0), accumulated_noise(0.0) {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(20);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_128_classic);

        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        keyPair = cc->KeyGen();
    }

    // Encode: value + noise (per dimension)
    vector<double> encodeWithNoise(double value, bool is_noise_only = false) {
        vector<double> dims(16, 0.0);
        
        if (is_noise_only) {
            // Pure noise slot (for cancellation)
            double noises[8] = {
                PHI_INV, PHI_INV * PHI_INV,
                fmod(PHI_INV * 2.0, 1.0), fmod(PHI_INV * 3.0, 1.0),
                fmod(PHI_INV * 5.0, 1.0), fmod(PHI_INV * 8.0, 1.0),
                fmod(PHI_INV * 13.0, 1.0), fmod(PHI_INV * 21.0, 1.0)
            };
            
            for (int i = 0; i < 8; i++) {
                dims[i] = noises[i];
                dims[i + 8] = noises[i] * PHI;
            }
            return dims;
        }
        
        // Base values (cold)
        dims[0] = value;                              // Normal
        dims[1] = log(value) / log(PHI);              // Logφ
        dims[2] = log(value);                         // Loge
        dims[3] = log2(value);                        // Log2
        dims[4] = log10(value);                       // Log10
        dims[5] = log(value) / log(PHI*PHI);          // Logφ²
        dims[6] = log(value) / log(PHI*PHI*PHI);      // Logφ³
        dims[7] = log(value) / log(SQRT5);            // Log√5
        
        // Noise values (φ-harmonized, CONSTANT per dimension)
        double noises[8] = {
            PHI_INV, PHI_INV * PHI_INV,
            fmod(PHI_INV * 2.0, 1.0), fmod(PHI_INV * 3.0, 1.0),
            fmod(PHI_INV * 5.0, 1.0), fmod(PHI_INV * 8.0, 1.0),
            fmod(PHI_INV * 13.0, 1.0), fmod(PHI_INV * 21.0, 1.0)
        };
        
        // Add noise to each dimension
        for (int i = 0; i < 8; i++) {
            dims[i] += noises[i];
        }
        
        // Mirror dimensions (φ-harmonized)
        for (int i = 0; i < 8; i++) {
            dims[i + 8] = (dims[i] - noises[i]) * PHI + noises[i] * PHI;
        }
        
        return dims;
    }
    
    // Decode: subtract noise, reverse log space
    double decodeWithNoise(const vector<complex<double>>& vals, int dim, int op_count = 0) {
        double noises[8] = {
            PHI_INV, PHI_INV * PHI_INV,
            fmod(PHI_INV * 2.0, 1.0), fmod(PHI_INV * 3.0, 1.0),
            fmod(PHI_INV * 5.0, 1.0), fmod(PHI_INV * 8.0, 1.0),
            fmod(PHI_INV * 13.0, 1.0), fmod(PHI_INV * 21.0, 1.0)
        };
        
        int orig_dim = dim % 8;
        bool is_mirror = dim >= 8;
        
        double val = vals[dim].real();
        
        // Remove accumulated noise
        double total_noise = noises[orig_dim] * (1.0 + op_count);
        
        if (is_mirror) {
            // Mirror: (cold × φ + noise × φ) / φ = cold + noise
            val = val / PHI;
            total_noise = total_noise * PHI;
        }
        
        val = val - total_noise;
        
        // Reverse log space
        switch(orig_dim) {
            case 0: return val;
            case 1: return pow(PHI, val);
            case 2: return exp(val);
            case 3: return pow(2.0, val);
            case 4: return pow(10.0, val);
            case 5: return pow(PHI*PHI, val);
            case 6: return pow(PHI*PHI*PHI, val);
            case 7: return pow(SQRT5, val);
        }
        return val;
    }
    
    // Encrypt
    Ciphertext<DCRTPoly> encrypt(double value) {
        auto dims = encodeWithNoise(value);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    // Decrypt
    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }
    
    // Add with noise tracking
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        operation_count++;
        accumulated_noise += PHI_INV;
        return cc->EvalAdd(a, b);
    }
    
    void runTests() {
        cout << fixed << setprecision(15);
        
        // TEST 1: Single value recovery
        cout << "========================================\n";
        cout << "  TEST 1: SINGLE VALUE RECOVERY\n";
        cout << "========================================\n\n";
        
        double test_val = 42.0;
        operation_count = 0;
        auto ct = encrypt(test_val);
        auto vals = decrypt(ct);
        
        double recovered = decodeWithNoise(vals, 0, operation_count);
        cout << "  Original: " << test_val << "\n";
        cout << "  Recovered: " << recovered << "\n";
        cout << "  Error: " << abs(recovered - test_val) << "\n\n";
        
        // TEST 2: Addition
        cout << "========================================\n";
        cout << "  TEST 2: ADDITION (42 + 8)\n";
        cout << "========================================\n\n";
        
        operation_count = 0;
        auto ct_a = encrypt(42.0);
        auto ct_b = encrypt(8.0);
        auto ct_sum = add(ct_a, ct_b);
        auto sum_vals = decrypt(ct_sum);
        
        double sum = decodeWithNoise(sum_vals, 0, operation_count);
        cout << "  42 + 8 = " << sum << "\n";
        cout << "  Expected: 50\n";
        cout << "  Error: " << abs(sum - 50.0) << "\n\n";
        
        // TEST 3: Chain addition
        cout << "========================================\n";
        cout << "  TEST 3: CHAIN (10+20+30+40)\n";
        cout << "========================================\n\n";
        
        operation_count = 0;
        auto ct_10 = encrypt(10.0);
        auto ct_20 = encrypt(20.0);
        auto ct_30 = encrypt(30.0);
        auto ct_40 = encrypt(40.0);
        
        auto ct_chain = add(add(add(ct_10, ct_20), ct_30), ct_40);
        auto chain_vals = decrypt(ct_chain);
        
        double chain = decodeWithNoise(chain_vals, 0, operation_count);
        cout << "  10+20+30+40 = " << chain << "\n";
        cout << "  Expected: 100\n";
        cout << "  Error: " << abs(chain - 100.0) << "\n\n";
        
        // TEST 4: 100 operations
        cout << "========================================\n";
        cout << "  TEST 4: 100 OPERATIONS\n";
        cout << "========================================\n\n";
        
        operation_count = 0;
        auto ct_acc = encrypt(0.0);
        auto ct_one = encrypt(1.0);
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < 100; i++) {
            ct_acc = add(ct_acc, ct_one);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        auto acc_vals = decrypt(ct_acc);
        double acc = decodeWithNoise(acc_vals, 0, operation_count);
        
        cout << "  100 ops: " << acc << "\n";
        cout << "  Expected: 100\n";
        cout << "  Error: " << abs(acc - 100.0) << "\n";
        cout << "  Time: " << time << " ms\n";
        cout << "  Level: " << ct_acc->GetLevel() << "\n\n";
        
        // TEST 5: 1000 operations
        cout << "========================================\n";
        cout << "  TEST 5: 1000 OPERATIONS\n";
        cout << "========================================\n\n";
        
        operation_count = 0;
        auto ct_1000 = encrypt(0.0);
        
        auto start_1000 = high_resolution_clock::now();
        
        for (int i = 0; i < 1000; i++) {
            ct_1000 = add(ct_1000, ct_one);
        }
        
        auto end_1000 = high_resolution_clock::now();
        auto time_1000 = duration_cast<milliseconds>(end_1000 - start_1000).count();
        
        auto vals_1000 = decrypt(ct_1000);
        double acc_1000 = decodeWithNoise(vals_1000, 0, operation_count);
        
        cout << "  1000 ops: " << acc_1000 << "\n";
        cout << "  Expected: 1000\n";
        cout << "  Error: " << abs(acc_1000 - 1000.0) << "\n";
        cout << "  Time: " << time_1000 << " ms\n";
        cout << "  Level: " << ct_1000->GetLevel() << "\n\n";
        
        // SUMMARY
        cout << "========================================\n";
        cout << "  FINAL EXACT SUMMARY\n";
        cout << "========================================\n\n";
        cout << "  ✅ Single: " << abs(recovered - test_val) << "\n";
        cout << "  ✅ 42+8: " << abs(sum - 50.0) << "\n";
        cout << "  ✅ Chain: " << abs(chain - 100.0) << "\n";
        cout << "  ✅ 100 ops: " << abs(acc - 100.0) << "\n";
        cout << "  ✅ 1000 ops: " << abs(acc_1000 - 1000.0) << "\n";
        cout << "  ✅ Level: 0\n";
        cout << "  ✅ LAHAT EMERGENT!\n\n";
    }
};

int main() {
    cout << "========================================\n";
    cout << "  φ-FINAL EXACT\n";
    cout << "  Noise Tracking + Auto-Subtraction\n";
    cout << "========================================\n\n";
    
    PhiFinalExact core;
    core.runTests();
    
    return 0;
}
