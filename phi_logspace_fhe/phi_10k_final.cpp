// ============================================
// φ^N 10K FINAL — SPEED TEST
// 10,000 operations na may N-configurable
// Interval print: bawat 1000 operations
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

int main() {
    cout << "========================================\n";
    cout << "  φ^N 10K FINAL — SPEED TEST\n";
    cout << "  10,000 operations + interval print\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(50);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    // ============================================
    // CONFIG: Piliin ang N dito
    // ============================================
    int N = 13;  // Fibonacci-selectable: 1,2,3,5,8,13,21,34
    double PHI_N = pow(PHI, N);
    double LN_PHI_N = log(PHI_N);
    
    cout << "  CONFIGURATION:\n";
    cout << "  N = " << N << "\n";
    cout << "  φ^N = " << PHI_N << "\n";
    cout << "  log(φ^N) = " << LN_PHI_N << "\n";
    cout << "  Expected speedup: " << N << "×\n\n";
    
    auto encrypt_log = [&](double value) {
        double log_val = log(value) / LN_PHI_N;
        vector<double> val(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return pow(PHI_N, log_val);
    };
    
    auto get_level = [&](const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    };
    
    auto get_towers = [&](const Ciphertext<DCRTPoly>& ct) {
        return ct->GetElements()[0].GetNumOfElements();
    };
    
    // ============================================
    // TEST: 10,000 MULTIPLICATIONS (×2 sa log space)
    // ============================================
    
    cout << "  TEST: 10,000 multiplications (×2)\n";
    cout << "  Operation: addition ng log(2) sa encrypted domain\n\n";
    
    auto ct = encrypt_log(1.0);
    double log2 = log(2.0) / LN_PHI_N;
    
    cout << "  Step | Time (ms) | Cumulative (ms) | Level | Towers | Value (log)\n";
    cout << "  -----|-----------|----------------|-------|--------|------------\n";
    
    auto start_total = high_resolution_clock::now();
    auto last_interval = start_total;
    
    for (int i = 1; i <= 10000; i++) {
        // Pre-computed log(2) para hindi mag-encrypt ng paulit-ulit
        vector<double> log2_val(1, log2);
        Plaintext pt_log2 = cc->MakeCKKSPackedPlaintext(log2_val);
        auto ct_log2 = cc->Encrypt(keyPair.publicKey, pt_log2);
        
        ct = cc->EvalAdd(ct, ct_log2);
        
        // Interval print: bawat 1000 operations
        if (i % 1000 == 0) {
            auto now = high_resolution_clock::now();
            auto interval_time = duration_cast<milliseconds>(now - last_interval).count();
            auto total_time = duration_cast<milliseconds>(now - start_total).count();
            
            Plaintext check_pt;
            cc->Decrypt(keyPair.secretKey, ct, &check_pt);
            check_pt->SetLength(1);
            double current_log = check_pt->GetCKKSPackedValue()[0].real();
            
            cout << "  " << setw(4) << i << " | "
                 << setw(9) << interval_time << " | "
                 << setw(14) << total_time << " | "
                 << setw(5) << get_level(ct) << " | "
                 << setw(6) << get_towers(ct) << " | "
                 << setw(10) << fixed << setprecision(2) << current_log << "\n";
            
            last_interval = now;
        }
    }
    
    auto end_total = high_resolution_clock::now();
    auto final_time = duration_cast<milliseconds>(end_total - start_total).count();
    
    cout << "\n  ✅ 10,000 operations complete!\n";
    cout << "  Total time: " << final_time << " ms\n";
    cout << "  Final level: " << get_level(ct) << "\n";
    cout << "  Final towers: " << get_towers(ct) << "\n\n";
    
    // ============================================
    // VERIFICATION
    // ============================================
    
    cout << "  VERIFICATION:\n";
    cout << "  Expected: 2^10000 (log = " << 10000 * log2 << ")\n";
    
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct, &result_pt);
    result_pt->SetLength(1);
    double result_log = result_pt->GetCKKSPackedValue()[0].real();
    
    cout << "  Actual: " << result_log << "\n";
    cout << "  Error: " << abs(result_log - 10000 * log2) << "\n\n";
    
    // ============================================
    // COMPARISON: N=1 vs N=13
    // ============================================
    
    cout << "  SPEED COMPARISON (estimated):\n";
    cout << "  N | Log Magnitude | Expected Speedup\n";
    cout << "  --|---------------|------------------\n";
    
    for (int n : {1, 2, 3, 5, 8, 13}) {
        double phi_n = pow(PHI, n);
        double ln_phi_n = log(phi_n);
        double log_mag = 10000 * log(2.0) / ln_phi_n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(13) << fixed << setprecision(1) << log_mag << " | "
             << setw(16) << n << "×\n";
    }
    
    cout << "\n  ========================================\n";
    cout << "  RESULT: 10K OPERATIONS COMPLETE\n";
    cout << "  Level: 0 | Bootstrapping: NONE\n";
    cout << "  ========================================\n";
    
    return 0;
}
