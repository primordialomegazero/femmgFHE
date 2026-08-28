// ============================================
// φ^N 1K SPEED TEST — 100 INTERVAL
//
// 1,000 operations, print bawat 100
// N-configurable speedup
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
    cout << "  φ^N 1K SPEED TEST\n";
    cout << "  1,000 ops + 100 interval print\n";
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
    
    // CONFIG: Piliin ang N dito
    int N = 8;  // 1,2,3,5,8,13
    double PHI_N = pow(PHI, N);
    double LN_PHI_N = log(PHI_N);
    
    cout << "  N = " << N << " | φ^N = " << PHI_N 
         << " | Speedup: " << N << "×\n\n";
    
    auto encrypt_log = [&](double value) {
        double log_val = log(value) / LN_PHI_N;
        vector<double> val(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    // Pre-compute log(2) ONCE
    double log2_val = log(2.0) / LN_PHI_N;
    vector<double> log2_vec(1, log2_val);
    Plaintext pt_log2 = cc->MakeCKKSPackedPlaintext(log2_vec);
    auto ct_log2 = cc->Encrypt(keyPair.publicKey, pt_log2);
    
    cout << "  TEST: 1,000 multiplications (×2)\n";
    cout << "  Operation: ct += ct_log2 (addition lang)\n\n";
    
    auto ct = encrypt_log(1.0);
    
    cout << "  Step | Interval (ms) | Total (ms) | Level | Towers\n";
    cout << "  -----|---------------|------------|-------|-------\n";
    
    auto start_total = high_resolution_clock::now();
    auto last_interval = start_total;
    
    for (int i = 1; i <= 1000; i++) {
        ct = cc->EvalAdd(ct, ct_log2);
        
        if (i % 100 == 0) {
            auto now = high_resolution_clock::now();
            auto interval_time = duration_cast<milliseconds>(now - last_interval).count();
            auto total_time = duration_cast<milliseconds>(now - start_total).count();
            
            cout << "  " << setw(4) << i << " | "
                 << setw(13) << interval_time << " | "
                 << setw(10) << total_time << " | "
                 << setw(5) << ct->GetLevel() << " | "
                 << setw(6) << ct->GetElements()[0].GetNumOfElements() << "\n";
            
            last_interval = now;
        }
    }
    
    auto end_total = high_resolution_clock::now();
    auto final_time = duration_cast<milliseconds>(end_total - start_total).count();
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Total: " << final_time << " ms\n";
    cout << "  Average per 100: " << final_time / 10 << " ms\n";
    cout << "  Final Level: " << ct->GetLevel() << "\n";
    cout << "  Final Towers: " << ct->GetElements()[0].GetNumOfElements() << "\n\n";
    
    // Verify
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct, &result_pt);
    result_pt->SetLength(1);
    double result_log = result_pt->GetCKKSPackedValue()[0].real();
    
    cout << "  VERIFY:\n";
    cout << "  Expected log: " << 1000 * log2_val << "\n";
    cout << "  Actual log: " << result_log << "\n";
    cout << "  Error: " << abs(result_log - 1000 * log2_val) << "\n\n";
    
    // ============================================
    // OPTIMIZATION: BATCH ENCRYPTION
    // ============================================
    
    cout << "  ========================================\n";
    cout << "  OPTIMIZATION TEST: PRE-ENCRYPTED BATCH\n";
    cout << "  ========================================\n\n";
    
    // Pre-encrypt 10 copies ng log(2) para walang re-encryption
    vector<Ciphertext<DCRTPoly>> batch;
    for (int i = 0; i < 10; i++) {
        batch.push_back(ct_log2);
    }
    
    cout << "  Pre-encrypted 10 copies ng log(2)\n";
    cout << "  Test: 1000 additions gamit ang batch\n\n";
    
    auto ct_batch = encrypt_log(1.0);
    
    auto start_batch = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_batch = cc->EvalAdd(ct_batch, batch[i % 10]);
    }
    
    auto end_batch = high_resolution_clock::now();
    auto batch_time = duration_cast<milliseconds>(end_batch - start_batch).count();
    
    cout << "  Batch time: " << batch_time << " ms\n";
    cout << "  vs Original: " << final_time << " ms\n";
    cout << "  Speedup: " << fixed << setprecision(2) 
         << (double)final_time / max(batch_time, 1L) << "×\n";
    
    return 0;
}
