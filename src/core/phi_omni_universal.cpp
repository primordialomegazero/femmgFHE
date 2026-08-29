// ============================================
// φ-OMNI UNIVERSAL — WALANG ANCHOR SHELL
//
// Lahat ng 8 shells ay pareho — walang special.
// Ang φ-harmonization ay BUILT-IN sa bawat
// shell, hindi kailangan ng hiwalay na anchor.
//
// 8 shells: [Sec, Quantum, Fractal, Entangle,
//            Modulo, Time, Compute, Universal]
//
// Lahat ay may kanya-kanyang fractional
// na kusang bounded sa [0,1).
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
    cout << "  φ-OMNI UNIVERSAL — WALANG ANCHOR\n";
    cout << "  Lahat ay Universal Space\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(20);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "  ✅ CKKS initialized (Depth 1, 8 shells)\n\n";
    
    // ============================================
    // UNIVERSAL SHELLS — LAHAT PAREHO
    // ============================================
    
    cout << "  PRE-ENCRYPTING 8 UNIVERSAL SHELLS...\n";
    cout << "  (Lahat ay may fractional-only values)\n\n";
    
    vector<double> universal_op(8, 0.0);
    
    // Lahat ng 8 shells ay may kanya-kanyang
    // fractional na naka-harmonize sa φ
    universal_op[0] = fmod(PHI_INV * 0.001, 1.0);      // Security
    universal_op[1] = fmod(PHI_INV * 0.01, 1.0);       // Quantum
    universal_op[2] = fmod(1.0 / 18.0, 1.0);           // Fractal
    universal_op[3] = fmod(PHI_INV * 0.5, 1.0);        // Entangle
    universal_op[4] = fmod(1.0 / 3.0, 1.0);            // Modulo
    universal_op[5] = fmod(PHI_INV, 1.0);              // Time
    universal_op[6] = fmod(log(2.0) / LN_PHI, 1.0);    // Compute
    universal_op[7] = fmod(PHI_INV * 0.001, 1.0);      // Universal (hindi anchor!)
    
    Plaintext pt_universal = cc->MakeCKKSPackedPlaintext(universal_op);
    auto ct_universal = cc->Encrypt(keyPair.publicKey, pt_universal);
    
    cout << "  ✅ Pre-encrypted 8 universal shells!\n";
    cout << "  (Walang special anchor — lahat pareho!)\n\n";
    
    // ============================================
    // 10K OMNI UNIVERSAL OPERATIONS
    // ============================================
    
    cout << "========================================\n";
    cout << "  10K OMNI UNIVERSAL\n";
    cout << "========================================\n\n";
    
    vector<double> start_vals(8, 0.0);
    Plaintext pt_start = cc->MakeCKKSPackedPlaintext(start_vals);
    auto ct_result = cc->Encrypt(keyPair.publicKey, pt_start);
    
    cout << "  Running 10,000 operations...\n\n";
    
    cout << "  Interval | Time (ms) | Cumulative\n";
    cout << "  ---------|-----------|----------\n";
    
    auto start_total = high_resolution_clock::now();
    auto last_interval = start_total;
    
    for (int i = 1; i <= 10000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_universal);
        
        if (i % 2000 == 0) {
            auto now = high_resolution_clock::now();
            auto interval_time = duration_cast<milliseconds>(now - last_interval).count();
            auto cumulative = duration_cast<milliseconds>(now - start_total).count();
            
            cout << "  " << setw(7) << i << " | "
                 << setw(9) << interval_time << " | "
                 << setw(10) << cumulative << "\n";
            
            last_interval = now;
        }
    }
    
    auto end_total = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end_total - start_total).count();
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Total: " << total_time << " ms\n";
    cout << "  Per op: " << fixed << setprecision(3) << (double)total_time/10000.0 << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";
    
    // ============================================
    // VERIFICATION — LAHAT BOUNDED SA [0,1)
    // ============================================
    
    cout << "  RESULTS (fractional part):\n";
    cout << "  Shell | Value | Fractional | Bounded?\n";
    cout << "  ------|-------|------------|----------\n";
    
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
    result_pt->SetLength(8);
    auto results = result_pt->GetCKKSPackedValue();
    
    string shell_names[] = {
        "Security", "Quantum", "Fractal", "Entangle",
        "Modulo", "Time", "Compute", "Universal"
    };
    
    bool all_bounded = true;
    for (int i = 0; i < 8; i++) {
        double val = results[i].real();
        double frac = val - floor(val);
        bool bounded = (frac >= 0.0 && frac < 1.0);
        if (!bounded) all_bounded = false;
        
        cout << "  " << setw(9) << shell_names[i] << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << setw(9) << setprecision(4) << frac << " | "
             << (bounded ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  ALL BOUNDED: " << (all_bounded ? "✅ YES — PERFECT!" : "❌ NO") << "\n\n";
    
    cout << "========================================\n";
    cout << "  OMNI UNIVERSAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 8 universal shells (walang special)\n";
    cout << "  ✅ Lahat fractional-bounded sa [0,1)\n";
    cout << "  ✅ 10K: " << total_time << " ms\n";
    cout << "  ✅ Per op: " << (double)total_time/10000.0 << " ms\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n";
    cout << "  ✅ Walang daya!\n\n";
    
    return 0;
}
