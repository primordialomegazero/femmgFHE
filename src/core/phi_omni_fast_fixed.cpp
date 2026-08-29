// ============================================
// φ-OMNI N-LAYER FAST FIXED — BAKED MODULO
//
// Ang modulo ay naka-BAKE sa pre-encrypted
// values — bawat shell ay may sariling modulo
// sa kanyang pre-computed value.
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
    cout << "  φ-OMNI FAST FIXED — BAKED MODULO\n";
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
    
    cout << "  ✅ CKKS initialized (Depth 1)\n\n";
    
    // ============================================
    // BAKED MODULO: VALUES NA NAKA-MODULO NA
    // ============================================
    
    cout << "  PRE-ENCRYPTING NA MAY BAKED MODULO...\n\n";
    
    // Bawat shell ay may value na NAKA-MODULO na
    // para walang overflow sa 10K iterations
    vector<double> baked_op(8, 0.0);
    
    // Security: maliit na φ-scaled noise
    baked_op[0] = 0.001 * PHI_INV;
    
    // Quantum: quasi-periodic
    baked_op[1] = fmod(PHI_INV, 1.0);
    
    // Fractal: compression (maliit lang)
    baked_op[2] = 1.0 / 18.0;
    
    // Entangle: φ-weighted
    baked_op[3] = PHI_INV * 0.1;
    
    // Modulo: -1/3 (auto-reset)
    baked_op[4] = -1.0 / 3.0;
    
    // Time: φ-dilation (mod 1)
    baked_op[5] = fmod(PHI_INV, 1.0);
    
    // Compute: ×2 sa log (mod 1 para sa bounded)
    baked_op[6] = fmod(log(2.0) / LN_PHI, 1.0);
    
    // Anchor: φ⁰ (stable)
    baked_op[7] = 0.0;
    
    Plaintext pt_baked = cc->MakeCKKSPackedPlaintext(baked_op);
    auto ct_baked = cc->Encrypt(keyPair.publicKey, pt_baked);
    
    cout << "  ✅ Pre-encrypted with baked modulo!\n\n";
    
    // ============================================
    // 10K OMNI OPERATIONS
    // ============================================
    
    cout << "========================================\n";
    cout << "  10K OMNI (BAKED MODULO)\n";
    cout << "========================================\n\n";
    
    vector<double> start_vals(8, 0.0);
    Plaintext pt_start = cc->MakeCKKSPackedPlaintext(start_vals);
    auto ct_result = cc->Encrypt(keyPair.publicKey, pt_start);
    
    cout << "  Running 10,000 operations...\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_baked);
    }
    
    auto end = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end - start).count();
    
    cout << "  ✅ Complete!\n";
    cout << "  Total: " << total_time << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";
    
    // Verify
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
    result_pt->SetLength(8);
    auto results = result_pt->GetCKKSPackedValue();
    
    cout << "  RESULTS:\n";
    cout << "  Shell | Value | Bounded?\n";
    cout << "  ------|-------|----------\n";
    
    string shell_names[] = {
        "Security", "Quantum", "Fractal", "Entangle",
        "Modulo", "Time", "Compute", "Anchor"
    };
    
    bool all_bounded = true;
    for (int i = 0; i < 8; i++) {
        double val = results[i].real();
        bool bounded = abs(val) < 10.0;
        if (!bounded) all_bounded = false;
        
        cout << "  " << setw(8) << shell_names[i] << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << (bounded ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  All bounded: " << (all_bounded ? "✅ YES!" : "❌ NO") << "\n\n";
    
    return 0;
}
