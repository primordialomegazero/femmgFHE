// ============================================
// φ-OMNI N-LAYER FAST — TAMANG PARAMETERS
//
// Depth 1 (addition lang)
// Pre-encrypted 8-shell operation
// 10K operations na may interval print
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>
#include <random>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-OMNI N-LAYER FAST — 10K\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);     // ADDITION LANG
    parameters.SetScalingModSize(20);         // MAS MALIIT
    parameters.SetBatchSize(8);               // 8 SHELLS
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> noise_dist(-0.01, 0.01);
    
    cout << "  ✅ CKKS initialized (Depth 1, 128-bit, 8 shells)\n";
    cout << "  Shells: [Sec φ⁸] [Quantum φ⁶] [Fractal φ⁵] [Ent φ⁴]\n";
    cout << "          [Mod φ³] [Time φ²] [Comp φ¹] [Anchor φ⁰]\n\n";
    
    // ============================================
    // PRE-ENCRYPT ONCE (8 SHELLS COMBINED)
    // ============================================
    
    cout << "  Pre-encrypting 8-shell operation...\n\n";
    
    vector<double> combined_op(8, 0.0);
    combined_op[0] = 0.01 * pow(PHI_INV, 4);  // φ⁸ Security
    combined_op[1] = 0.01 * pow(PHI_INV, 3);  // φ⁶ Quantum
    combined_op[2] = 1.0 / 18.0;               // φ⁵ Fractal
    combined_op[3] = 0.5;                      // φ⁴ Entangle
    combined_op[4] = -1.0 / 3.0;               // φ³ Modulo
    combined_op[5] = PHI_INV;                  // φ² Time
    combined_op[6] = log(2.0) / LN_PHI;        // φ¹ Compute
    combined_op[7] = 0.0;                      // φ⁰ Anchor
    
    Plaintext pt_combined = cc->MakeCKKSPackedPlaintext(combined_op);
    auto ct_combined = cc->Encrypt(keyPair.publicKey, pt_combined);
    
    cout << "  ✅ Pre-encrypted ONCE!\n";
    cout << "  I-REUSE na lang sa bawat EvalAdd\n\n";
    
    // ============================================
    // 10K OMNI OPERATIONS
    // ============================================
    
    cout << "========================================\n";
    cout << "  10K OMNI N-LAYER OPERATIONS\n";
    cout << "========================================\n\n";
    
    vector<double> start_shells(8, 0.0);
    Plaintext pt_start = cc->MakeCKKSPackedPlaintext(start_shells);
    auto ct_result = cc->Encrypt(keyPair.publicKey, pt_start);
    
    cout << "  Starting 10,000 operations...\n";
    cout << "  (8 shells sabay-sabay sa bawat EvalAdd)\n\n";
    
    cout << "  Interval | Time (ms) | Cumulative (ms)\n";
    cout << "  ---------|-----------|----------------\n";
    
    auto start_total = high_resolution_clock::now();
    auto last_interval = start_total;
    
    for (int i = 1; i <= 10000; i++) {
        // REUSE ang pre-encrypted ct_combined
        ct_result = cc->EvalAdd(ct_result, ct_combined);
        
        if (i % 500 == 0) {
            auto now = high_resolution_clock::now();
            auto interval_time = duration_cast<milliseconds>(now - last_interval).count();
            auto cumulative = duration_cast<milliseconds>(now - start_total).count();
            
            cout << "  " << setw(7) << i << " | "
                 << setw(9) << interval_time << " | "
                 << setw(14) << cumulative << "\n";
            
            last_interval = now;
        }
    }
    
    auto end_total = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end_total - start_total).count();
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Total: " << total_time << " ms\n";
    cout << "  Per op: " << fixed << setprecision(3) << (double)total_time / 10000.0 << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";
    
    // ============================================
    // VERIFICATION (DECRYPT SA DULO)
    // ============================================
    
    cout << "  VERIFICATION (decrypt sa dulo lang):\n";
    
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
    result_pt->SetLength(8);
    auto results = result_pt->GetCKKSPackedValue();
    
    string shell_names[] = {
        "SECURITY (φ⁸)", "QUANTUM (φ⁶)", "FRACTAL (φ⁵)", "ENTANGLE (φ⁴)",
        "MODULO (φ³)", "TIME (φ²)", "COMPUTE (φ¹)", "ANCHOR (φ⁰)"
    };
    
    cout << "  Shell | Name | Value | Bounded?\n";
    cout << "  ------|------|-------|----------\n";
    
    for (int i = 0; i < 8; i++) {
        double val = results[i].real();
        bool bounded = abs(val) < 100.0;
        
        cout << "  " << setw(5) << i << " | "
             << setw(14) << shell_names[i] << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << (bounded ? "✅" : "❌") << "\n";
    }
    
    cout << "\n========================================\n";
    cout << "  OMNI FAST COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 8 shells sa 1 EvalAdd\n";
    cout << "  ✅ 10K: " << total_time << " ms\n";
    cout << "  ✅ Per op: " << (double)total_time/10000.0 << " ms\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Walang daya!\n\n";
    
    return 0;
}
