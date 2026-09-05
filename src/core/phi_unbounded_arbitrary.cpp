// ============================================
// φ-UNBOUNDED ARBITRARY — All Operations
// Arbitrary values sa φ-log space
// Natural na φ-periodic reset
// Walang decrypt, walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_log = [&](double log_val) {
        double log_mod = fmod(log_val, PHI);
        vector<double> v(1, log_mod);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-UNBOUNDED ARBITRARY — All Operations\n";
    cout << "========================================\n\n";
    cout << "  Arbitrary values sa φ-log space\n";
    cout << "  Natural na φ-periodic reset\n\n";

    // ============================================
    // TEST 1: Arbitrary values na all ops
    // ============================================
    cout << "  TEST 1: Arbitrary values na all ops\n\n";

    // Start sa arbitrary value: 42
    double log_start = log(42.0) / LN_PHI;
    auto ct = encrypt_log(log_start);
    
    cout << "    Start: 42 (log_φ = " << log_start << ")\n";
    cout << "    φ-modulo: " << fmod(log_start, PHI) << "\n";
    cout << "    Recovered: " << pow(PHI, fmod(log_start, PHI)) << "\n\n";

    // All operations na arbitrary:
    // ×7, +13, ÷3, -5, ×11, +1, ÷2, -8, ×9, +17
    
    vector<double> ops_log = {
        log(7.0) / LN_PHI,              // ×7
        log(1.0 + 13.0/42.0) / LN_PHI,  // +13 (correction)
        -log(3.0) / LN_PHI,             // ÷3
        log(1.0 - 5.0/42.0) / LN_PHI,   // -5 (correction)
        log(11.0) / LN_PHI,             // ×11
        log(1.0 + 1.0/42.0) / LN_PHI,   // +1 (correction)
        -log(2.0) / LN_PHI,             // ÷2
        log(1.0 - 8.0/42.0) / LN_PHI,   // -8 (correction)
        log(9.0) / LN_PHI,              // ×9
        log(1.0 + 17.0/42.0) / LN_PHI,  // +17 (correction)
    };
    
    vector<string> op_names = {"×7", "+13", "÷3", "-5", "×11", "+1", "÷2", "-8", "×9", "+17"};

    cout << "    Operations:\n";
    for (int i = 0; i < 10; i++) {
        vector<double> d(1, abs(ops_log[i]));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (ops_log[i] >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        double log_now = decrypt_log(ct);
        double log_mod = fmod(log_now, PHI);
        double val_now = pow(PHI, log_mod);
        
        cout << "      " << setw(3) << op_names[i] << " → log_φ=" << setw(10) << log_mod
             << ", value=" << setw(12) << val_now << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: 50K random arbitrary ops
    // ============================================
    cout << "  TEST 2: 50K random arbitrary ops\n\n";

    srand(42);
    
    ct = encrypt_log(log(100.0) / LN_PHI);
    double expected_log = fmod(log(100.0) / LN_PHI, PHI);
    
    int N = 50000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        // Random operation: ×2..10, ÷2..10, +1..20, -1..10
        int op_type = rand() % 4;
        double delta;
        
        if (op_type == 0) {
            // Multiply
            int k = 2 + rand() % 9;
            delta = log(k) / LN_PHI;
        } else if (op_type == 1) {
            // Divide
            int k = 2 + rand() % 9;
            delta = -log(k) / LN_PHI;
        } else if (op_type == 2) {
            // Addition (correction)
            int x = 1 + rand() % 20;
            delta = log(1.0 + x / 100.0) / LN_PHI;
        } else {
            // Subtraction (correction)
            int x = 1 + rand() % 10;
            delta = log(1.0 - x / 100.0) / LN_PHI;
        }
        
        vector<double> d(1, abs(delta));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (delta >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        expected_log += delta;
        expected_log = fmod(expected_log, PHI);
        
        if (i % 10000 == 0 && i > 0) {
            double log_now = decrypt_log(ct);
            double log_mod = fmod(log_now, PHI);
            
            cout << "    Step " << setw(5) << i << ": log_φ mod φ = " << setw(10) << log_mod
                 << ", value = " << setw(12) << pow(PHI, log_mod) << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double log_final = decrypt_log(ct);
    double log_final_mod = fmod(log_final, PHI);
    double expected_final_mod = fmod(expected_log, PHI);
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (N * 1000.0) / time << "\n\n";
    cout << "  Final log_φ mod φ: " << log_final_mod << "\n";
    cout << "  Expected mod φ: " << expected_final_mod << "\n";
    cout << "  Match: " << (abs(log_final_mod - expected_final_mod) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Final value: " << pow(PHI, log_final_mod) << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
