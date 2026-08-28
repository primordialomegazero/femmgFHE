// ============================================
// φ-NOISE MEASUREMENT SA ACTUAL CKKS
//
// Hindi simulation. Actual CKKS noise measurement.
// Compare φ-based operations vs standard.
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace std;
using namespace std::chrono;
using namespace lbcrypto;

class PhiNoiseMeasurement {
private:
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    CryptoContext<DCRTPoly> cryptoContext;
    KeyPair<DCRTPoly> keyPair;
    
    // Noise measurement via decryption error
    struct NoiseResult {
        int depth;
        double noise_magnitude;
        double relative_error;
        bool valid;
    };
    
public:
    PhiNoiseMeasurement() {
        cout << "========================================\n";
        cout << "  φ-NOISE MEASUREMENT SA ACTUAL CKKS\n";
        cout << "========================================\n\n";
        
        uint32_t multDepth = 30;
        uint32_t scaleModSize = 50;
        uint32_t batchSize = 8;
        
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(scaleModSize);
        parameters.SetBatchSize(batchSize);
        
        cryptoContext = GenCryptoContext(parameters);
        cryptoContext->Enable(PKE);
        cryptoContext->Enable(KEYSWITCH);
        cryptoContext->Enable(LEVELEDSHE);
        
        keyPair = cryptoContext->KeyGen();
        cryptoContext->EvalMultKeyGen(keyPair.secretKey);
        
        cout << "  ✅ CKKS initialized\n";
        cout << "  Multiplicative depth: " << multDepth << "\n";
        cout << "  Scale mod size: " << scaleModSize << " bits\n\n";
    }
    
    // ============================================
    // TEST 1: STANDARD CKKS NOISE GROWTH
    // ============================================
    
    vector<NoiseResult> test_standard_noise() {
        cout << "========================================\n";
        cout << "  TEST 1: STANDARD CKKS NOISE GROWTH\n";
        cout << "========================================\n\n";
        
        vector<NoiseResult> results;
        
        // Encrypt value 1.0
        vector<double> values = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
        Plaintext pt = cryptoContext->MakeCKKSPackedPlaintext(values);
        auto ct = cryptoContext->Encrypt(keyPair.publicKey, pt);
        
        cout << "  Multiplying by 1.001 repeatedly...\n\n";
        
        for (int depth = 0; depth <= 25; depth++) {
            if (depth > 0) {
                ct = cryptoContext->EvalMult(ct, 1.001);
            }
            
            // Measure noise via decryption
            Plaintext result_pt;
            cryptoContext->Decrypt(keyPair.secretKey, ct, &result_pt);
            result_pt->SetLength(8);
            
            double result = result_pt->GetCKKSPackedValue()[0].real();
            double expected = pow(1.001, depth);
            double error = abs(result - expected);
            
            NoiseResult nr;
            nr.depth = depth;
            nr.noise_magnitude = error;
            nr.relative_error = error / max(abs(expected), 1e-10);
            nr.valid = (nr.relative_error < 0.01);
            
            results.push_back(nr);
            
            if (depth % 5 == 0) {
                cout << "  Depth " << setw(2) << depth << ": "
                     << "value=" << fixed << setprecision(6) << result
                     << ", error=" << scientific << setprecision(2) << error
                     << ", valid=" << (nr.valid ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  STANDARD NOISE SUMMARY:\n";
        cout << "  Max depth before invalidity: ";
        for (auto& r : results) {
            if (!r.valid) {
                cout << r.depth << "\n";
                break;
            }
        }
        cout << "\n";
        
        return results;
    }
    
    // ============================================
    // TEST 2: φ-BASED ADDITIVE NOISE
    // ============================================
    
    vector<NoiseResult> test_phi_additive_noise() {
        cout << "========================================\n";
        cout << "  TEST 2: φ-BASED ADDITIVE (FIBONACCI)\n";
        cout << "========================================\n\n";
        
        vector<NoiseResult> results;
        
        // Encrypt F_0 = 0, F_1 = 1
        vector<double> F0 = {0, 0, 0, 0, 0, 0, 0, 0};
        vector<double> F1 = {1, 1, 1, 1, 1, 1, 1, 1};
        
        Plaintext pt0 = cryptoContext->MakeCKKSPackedPlaintext(F0);
        Plaintext pt1 = cryptoContext->MakeCKKSPackedPlaintext(F1);
        
        auto ct_n = cryptoContext->Encrypt(keyPair.publicKey, pt0);
        auto ct_n_plus_1 = cryptoContext->Encrypt(keyPair.publicKey, pt1);
        
        cout << "  Running Fibonacci sa encrypted domain...\n\n";
        
        // Track F values for comparison
        vector<double> fib = {0, 1};
        
        for (int depth = 0; depth <= 50; depth++) {
            if (depth >= 2) {
                // F_{n} = F_{n-1} + F_{n-2}
                auto ct_next = cryptoContext->EvalAdd(ct_n_plus_1, ct_n);
                ct_n = ct_n_plus_1;
                ct_n_plus_1 = ct_next;
                
                fib.push_back(fib[depth-1] + fib[depth-2]);
            }
            
            // Measure noise
            Plaintext result_pt;
            cryptoContext->Decrypt(keyPair.secretKey, ct_n_plus_1, &result_pt);
            result_pt->SetLength(8);
            
            double result = result_pt->GetCKKSPackedValue()[0].real();
            double expected = fib[depth];
            double error = abs(result - expected);
            
            NoiseResult nr;
            nr.depth = depth;
            nr.noise_magnitude = error;
            nr.relative_error = error / max(abs(expected), 1e-10);
            nr.valid = (nr.relative_error < 0.01);
            
            results.push_back(nr);
            
            if (depth % 10 == 0) {
                cout << "  Depth " << setw(2) << depth << ": "
                     << "F_" << depth << "=" << fixed << setprecision(0) << result
                     << ", error=" << scientific << setprecision(2) << error
                     << ", valid=" << (nr.valid ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  φ-ADDITIVE NOISE SUMMARY:\n";
        cout << "  All depths valid: ";
        bool all_valid = true;
        for (auto& r : results) {
            if (!r.valid) {
                all_valid = false;
                break;
            }
        }
        cout << (all_valid ? "✅ YES" : "❌ NO") << "\n\n";
        
        return results;
    }
    
    // ============================================
    // TEST 3: φ-MULTIPLICATIVE NOISE
    // ============================================
    
    vector<NoiseResult> test_phi_multiplicative_noise() {
        cout << "========================================\n";
        cout << "  TEST 3: φ-MULTIPLICATIVE NOISE\n";
        cout << "========================================\n\n";
        
        vector<NoiseResult> results;
        
        // Encrypt φ values
        vector<double> phi_values = {PHI, PHI_INV, PHI, PHI_INV, PHI, PHI_INV, PHI, PHI_INV};
        Plaintext pt = cryptoContext->MakeCKKSPackedPlaintext(phi_values);
        auto ct = cryptoContext->Encrypt(keyPair.publicKey, pt);
        
        cout << "  Multiplying by φ repeatedly...\n\n";
        
        for (int depth = 0; depth <= 20; depth++) {
            if (depth > 0) {
                ct = cryptoContext->EvalMult(ct, PHI);
            }
            
            // Measure noise
            Plaintext result_pt;
            cryptoContext->Decrypt(keyPair.secretKey, ct, &result_pt);
            result_pt->SetLength(8);
            
            double result = result_pt->GetCKKSPackedValue()[0].real();
            double expected = pow(PHI, depth);
            double error = abs(result - expected);
            
            NoiseResult nr;
            nr.depth = depth;
            nr.noise_magnitude = error;
            nr.relative_error = error / max(abs(expected), 1e-10);
            nr.valid = (nr.relative_error < 0.01);
            
            results.push_back(nr);
            
            if (depth % 3 == 0) {
                cout << "  Depth " << setw(2) << depth << ": "
                     << "φ^" << depth << "=" << fixed << setprecision(4) << result
                     << ", error=" << scientific << setprecision(2) << error
                     << ", valid=" << (nr.valid ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  φ-MULTIPLICATIVE NOISE SUMMARY:\n";
        cout << "  Max depth before invalidity: ";
        for (auto& r : results) {
            if (!r.valid) {
                cout << r.depth << "\n";
                break;
            }
        }
        cout << "\n";
        
        return results;
    }
    
    // ============================================
    // TEST 4: DIRECT COMPARISON
    // ============================================
    
    void test_direct_comparison() {
        cout << "========================================\n";
        cout << "  TEST 4: DIRECT NOISE COMPARISON\n";
        cout << "========================================\n\n";
        
        cout << "  Same depth, same operations.\n";
        cout << "  Standard vs φ-based.\n\n";
        
        // Standard: multiply by 2 repeatedly
        vector<double> std_values = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
        Plaintext std_pt = cryptoContext->MakeCKKSPackedPlaintext(std_values);
        auto std_ct = cryptoContext->Encrypt(keyPair.publicKey, std_pt);
        
        // φ-based: multiply by φ repeatedly  
        vector<double> phi_values = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
        Plaintext phi_pt = cryptoContext->MakeCKKSPackedPlaintext(phi_values);
        auto phi_ct = cryptoContext->Encrypt(keyPair.publicKey, phi_pt);
        
        cout << "  Depth | Std Error | φ Error | φ/Std Ratio | Better?\n";
        cout << "  ------|-----------|---------|-------------|--------\n";
        
        for (int depth = 1; depth <= 15; depth++) {
            // Standard: multiply by 2
            std_ct = cryptoContext->EvalMult(std_ct, 2.0);
            
            // φ-based: multiply by φ
            phi_ct = cryptoContext->EvalMult(phi_ct, PHI);
            
            // Measure errors
            Plaintext std_result_pt, phi_result_pt;
            cryptoContext->Decrypt(keyPair.secretKey, std_ct, &std_result_pt);
            cryptoContext->Decrypt(keyPair.secretKey, phi_ct, &phi_result_pt);
            std_result_pt->SetLength(8);
            phi_result_pt->SetLength(8);
            
            double std_result = std_result_pt->GetCKKSPackedValue()[0].real();
            double phi_result = phi_result_pt->GetCKKSPackedValue()[0].real();
            
            double std_expected = pow(2.0, depth);
            double phi_expected = pow(PHI, depth);
            
            double std_error = abs(std_result - std_expected);
            double phi_error = abs(phi_result - phi_expected);
            
            double ratio = phi_error / max(std_error, 1e-30);
            bool better = phi_error < std_error;
            
            cout << "  " << setw(5) << depth << " | "
                 << setw(9) << scientific << setprecision(2) << std_error << " | "
                 << setw(7) << phi_error << " | "
                 << setw(11) << fixed << setprecision(4) << ratio << " | "
                 << (better ? "✅ φ" : "❌ Std") << "\n";
        }
        
        cout << "\n  FINDING:\n";
        cout << "  Kung φ-based ay may mas mababang error sa parehong depth,\n";
        cout << "  may actual advantage sa encrypted domain.\n";
        cout << "  Kung hindi, kailangan ng ibang approach.\n\n";
    }

public:
    void run_all() {
        auto standard_results = test_standard_noise();
        auto phi_additive_results = test_phi_additive_noise();
        auto phi_mult_results = test_phi_multiplicative_noise();
        test_direct_comparison();
        
        cout << "========================================\n";
        cout << "  NOISE MEASUREMENT COMPLETE\n";
        cout << "========================================\n\n";
        
        cout << "  SUMMARY:\n";
        cout << "  ✅ Standard CKKS noise measured\n";
        cout << "  ✅ φ-additive noise measured\n";
        cout << "  ✅ φ-multiplicative noise measured\n";
        cout << "  ✅ Direct comparison done\n\n";
        
        cout << "  KEY QUESTION:\n";
        cout << "  May φ-based operations ba na mas mababa\n";
        cout << "  ang noise kaysa standard sa parehong depth?\n\n";
    }
};

int main() {
    PhiNoiseMeasurement test;
    test.run_all();
    return 0;
}
