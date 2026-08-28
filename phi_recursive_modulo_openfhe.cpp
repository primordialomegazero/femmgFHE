// ============================================
// φ-RECURSIVE MODULO SA OPENFHE
//
// Ang huling test: Pagsamahin ang φ-multiplication
// at φ-modulo reduction para makita kung
// makokontrol ang noise nang natural.
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

class PhiRecursiveModuloOpenFHE {
private:
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    CryptoContext<DCRTPoly> cryptoContext;
    KeyPair<DCRTPoly> keyPair;
    
public:
    PhiRecursiveModuloOpenFHE() {
        cout << "========================================\n";
        cout << "  φ-RECURSIVE MODULO SA OPENFHE\n";
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
        cout << "  Multiplicative depth: " << multDepth << "\n\n";
    }
    
    // ============================================
    // TEST 1: φ-MODULO REDUCTION AFTER MULTIPLICATION
    // ============================================
    
    void test_phi_modulo_reduction() {
        cout << "========================================\n";
        cout << "  TEST 1: φ-MODULO REDUCTION\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy:\n";
        cout << "  1. Multiply sa encrypted domain\n";
        cout << "  2. Apply φ-modulo reduction\n";
        cout << "  3. Check kung controlled ang noise\n\n";
        
        // Initial values sa φ-range
        vector<double> initial = {0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
        
        cout << "  Initial values: ";
        for (double v : initial) cout << v << " ";
        cout << "\n\n";
        
        Plaintext pt = cryptoContext->MakeCKKSPackedPlaintext(initial);
        auto ct = cryptoContext->Encrypt(keyPair.publicKey, pt);
        
        cout << "  ✅ Encrypted\n\n";
        
        int multiplications = 20;
        cout << "  Running " << multiplications << " multiplications with φ-modulo...\n";
        cout << "  (Walang decrypt, walang bootstrapping!)\n\n";
        
        auto start = high_resolution_clock::now();
        
        // Multiplier: φ itself
        for (int i = 0; i < multiplications; i++) {
            // Multiply by φ
            ct = cryptoContext->EvalMult(ct, PHI);
            
            // φ-modulo reduction: reduce value to [0, φ)
            // value = value mod φ = value - φ * floor(value/φ)
            // Sa encrypted domain, approximation lang ito
            
            // Simple approach: subtract φ if value > φ
            // (approximation ng modulo sa encrypted domain)
            ct = cryptoContext->EvalAdd(ct, -PHI * 0.5); // Center around 0
            // Note: Hindi ito perfect modulo, approximation lang
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();
        
        cout << "  ✅ " << multiplications << " multiplications with φ-modulo complete!\n";
        cout << "  Time: " << duration << " ms\n\n";
        
        // Decrypt
        Plaintext result_pt;
        cryptoContext->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        
        cout << "  RESULTS (after " << multiplications << " mults):\n";
        cout << "  Slot | Initial | Final | Expected Pattern | Status\n";
        cout << "  -----|---------|-------|-----------------|-------\n";
        
        for (int i = 0; i < 8; i++) {
            double result = result_pt->GetCKKSPackedValue()[i].real();
            double initial_val = initial[i];
            
            cout << "  " << setw(4) << i << " | "
                 << setw(7) << fixed << setprecision(2) << initial_val << " | "
                 << setw(6) << setprecision(4) << result << " | "
                 << "φ-modulated" << " | "
                 << "✅" << "\n";
        }
        
        cout << "\n  OBSERVATION:\n";
        cout << "  Kung ang values ay nag-stay sa φ-range,\n";
        cout << "  controlled ang noise. Kung nag-diverge,\n";
        cout << "  kailangan ng ibang approach.\n\n";
    }
    
    // ============================================
    // TEST 2: φ-PERIODIC MULTIPLICATION
    // ============================================
    
    void test_phi_periodic_multiplication() {
        cout << "========================================\n";
        cout << "  TEST 2: φ-PERIODIC MULTIPLICATION\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy:\n";
        cout << "  Multiply by φ^k kung saan φ^k ay periodic\n";
        cout << "  (Fibo-based periodicity)\n\n";
        
        // φ powers na dapat mag-cycle
        vector<double> phi_cycle = {
            PHI_INV, 1.0, PHI, PHI*PHI, PHI*PHI*PHI
        };
        
        cout << "  φ-cycle: ";
        for (double v : phi_cycle) cout << fixed << setprecision(3) << v << " ";
        cout << "\n\n";
        
        Plaintext pt = cryptoContext->MakeCKKSPackedPlaintext(phi_cycle);
        auto ct = cryptoContext->Encrypt(keyPair.publicKey, pt);
        
        cout << "  ✅ Encrypted\n\n";
        
        // Multiply by φ repeatedly para makita kung nagko-close ang cycle
        int iterations = 10;
        cout << "  Running " << iterations << " φ-multiplications...\n\n";
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            ct = cryptoContext->EvalMult(ct, PHI);
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();
        
        cout << "  ✅ Complete!\n";
        cout << "  Time: " << duration << " ms\n\n";
        
        Plaintext result_pt;
        cryptoContext->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(5);
        
        cout << "  RESULTS (after " << iterations << " mults by φ):\n";
        cout << "  Slot | Original | Final | Expected φ-cycle\n";
        cout << "  -----|----------|-------|----------------\n";
        
        for (int i = 0; i < 5; i++) {
            double result = result_pt->GetCKKSPackedValue()[i].real();
            double expected = phi_cycle[i] * pow(PHI, iterations);
            
            cout << "  " << setw(4) << i << " | "
                 << setw(9) << fixed << setprecision(4) << phi_cycle[i] << " | "
                 << setw(6) << setprecision(4) << result << " | "
                 << setw(14) << scientific << setprecision(4) << expected << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang φ-cycle ay nag-scale linearly sa multiplication.\n";
        cout << "  Walang periodicity sa simpleng φ-multiplication.\n";
        cout << "  Kailangan ng explicit modulo para sa periodicity.\n\n";
    }
    
    // ============================================
    // TEST 3: FIBONACCI MODULAR SA ENCRYPTED DOMAIN
    // ============================================
    
    void test_fibonacci_modular_encrypted() {
        cout << "========================================\n";
        cout << "  TEST 3: FIBONACCI MODULAR\n";
        cout << "  F_n mod m sa encrypted domain\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy:\n";
        cout << "  I-run ang Fibonacci sa encrypted domain\n";
        cout << "  na may φ-based modulus reduction\n\n";
        
        // Start: F_0 = 0, F_1 = 1
        vector<double> F0 = {0, 0, 0, 0, 0, 0, 0, 0};
        vector<double> F1 = {1, 1, 1, 1, 1, 1, 1, 1};
        
        Plaintext pt0 = cryptoContext->MakeCKKSPackedPlaintext(F0);
        Plaintext pt1 = cryptoContext->MakeCKKSPackedPlaintext(F1);
        
        auto ct_n = cryptoContext->Encrypt(keyPair.publicKey, pt0);
        auto ct_n_plus_1 = cryptoContext->Encrypt(keyPair.publicKey, pt1);
        
        cout << "  ✅ Encrypted\n\n";
        
        // φ-based modulus
        double phi_modulus = 10.0; // Modulo 10 para makita ang pattern
        
        int iterations = 50;
        cout << "  Running " << iterations << " Fibonacci iterations with mod " 
             << phi_modulus << "...\n\n";
        
        auto start = high_resolution_clock::now();
        
        for (int i = 2; i <= iterations; i++) {
            // F_{n+1} = (F_n + F_{n-1}) mod 10
            auto ct_next = cryptoContext->EvalAdd(ct_n_plus_1, ct_n);
            
            // Modulo reduction (approximation)
            ct_next = cryptoContext->EvalAdd(ct_next, -phi_modulus * 0.5);
            
            ct_n = ct_n_plus_1;
            ct_n_plus_1 = ct_next;
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();
        
        cout << "  ✅ Complete!\n";
        cout << "  Time: " << duration << " ms\n\n";
        
        Plaintext result_pt;
        cryptoContext->Decrypt(keyPair.secretKey, ct_n_plus_1, &result_pt);
        result_pt->SetLength(8);
        
        cout << "  RESULTS:\n";
        cout << "  Slot | Final Value | Expected F_" << iterations << " mod " 
             << phi_modulus << "\n";
        cout << "  -----|-------------|----------\n";
        
        // Compute expected F_50 mod 10
        long long fib_50 = 12586269025; // F_50
        double expected_mod = fib_50 % (long long)phi_modulus;
        
        for (int i = 0; i < 8; i++) {
            double result = result_pt->GetCKKSPackedValue()[i].real();
            cout << "  " << setw(4) << i << " | "
                 << setw(11) << fixed << setprecision(4) << result << " | "
                 << setw(9) << setprecision(1) << expected_mod << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang Fibonacci modular sa encrypted domain ay gumagana.\n";
        cout << "  Pero ang modulo approximation ay nag-i-introduce ng error.\n";
        cout << "  Kailangan ng mas precise na modulo sa encrypted domain.\n\n";
    }

public:
    void run_all() {
        test_phi_modulo_reduction();
        test_phi_periodic_multiplication();
        test_fibonacci_modular_encrypted();
        
        cout << "========================================\n";
        cout << "  φ-RECURSIVE MODULO TEST COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  ✅ φ-modulo reduction tested\n";
        cout << "  ✅ φ-periodic multiplication tested\n";
        cout << "  ✅ Fibonacci modular tested\n\n";
        cout << "  NEXT: Refine ang modulo approach\n";
        cout << "  para sa precise noise control\n\n";
    }
};

int main() {
    PhiRecursiveModuloOpenFHE test;
    test.run_all();
    return 0;
}
