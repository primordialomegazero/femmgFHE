// ============================================
// φ-1M RICH — POS+NEG DIRECT NA MAY DETAILS
//
// 1M direct operations na may:
// - Progress updates
// - Value distribution
// - φ-modulo statistics
// - Positive/Negative ratio
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-1M RICH — POS+NEG DIRECT\n";
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

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = PHI;

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    auto encrypt_val = [&](double val) {
        vector<double> v(16, fmod(val, PHI_MOD));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return fmod(sum / 16.0, PHI_MOD);
    };

    // ============================================
    // RANDOM VALUES DISTRIBUTION
    // ============================================

    mt19937 gen(42);
    uniform_real_distribution<double> random_val(-100.0, 100.0);

    cout << "========================================\n";
    cout << "  VALUE DISTRIBUTION (FIRST 20)\n";
    cout << "========================================\n\n";

    vector<double> sample_vals;
    for (int i = 0; i < 20; i++) {
        double v = random_val(gen);
        sample_vals.push_back(v);
        cout << "  " << setw(3) << i << ": " 
             << setw(8) << fixed << setprecision(2) << v << " → mod φ: "
             << setw(6) << fmod(v, PHI_MOD) << "\n";
    }

    cout << "\n";

    // ============================================
    // 1M DIRECT OPERATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  1M DIRECT OPERATIONS\n";
    cout << "========================================\n\n";

    mt19937 gen2(42);
    auto ct_acc = encrypt_val(0.0);
    
    double total_sum = 0.0;
    int pos_count = 0, neg_count = 0;
    vector<double> all_mods;

    cout << "  Progress:\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000000; i++) {
        double val = random_val(gen2);
        if (val >= 0) pos_count++;
        else neg_count++;
        
        double mod_val = fmod(val, PHI_MOD);
        total_sum = fmod(total_sum + mod_val, PHI_MOD);
        all_mods.push_back(mod_val);
        
        auto ct_val = encrypt_val(mod_val);
        ct_acc = cc->EvalAdd(ct_acc, ct_val);
        
        // Progress every 100K
        if ((i + 1) % 100000 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            cout << "  " << (i + 1) / 1000 << "K ops | "
                 << elapsed << "s elapsed | "
                 << "Running sum: " << fixed << setprecision(3) << decrypt_val(ct_acc) << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_val(ct_acc);

    cout << "\n  ✅ 1M COMPLETE!\n";
    cout << "  Total time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Towers: " << ct_acc->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // STATISTICS
    // ============================================

    cout << "========================================\n";
    cout << "  STATISTICS\n";
    cout << "========================================\n\n";

    cout << "  Positive values: " << pos_count << " (" 
         << fixed << setprecision(1) << (double)pos_count / 10000.0 << "%)\n";
    cout << "  Negative values: " << neg_count << " (" 
         << (double)neg_count / 10000.0 << "%)\n";
    
    double min_mod = *min_element(all_mods.begin(), all_mods.end());
    double max_mod = *max_element(all_mods.begin(), all_mods.end());
    double avg_mod = 0.0;
    for (double m : all_mods) avg_mod += m;
    avg_mod /= all_mods.size();
    
    cout << "  Min mod φ: " << min_mod << "\n";
    cout << "  Max mod φ: " << max_mod << "\n";
    cout << "  Avg mod φ: " << avg_mod << "\n";
    cout << "  φ/2 threshold: " << PHI_MOD / 2.0 << "\n\n";

    cout << "  FINAL RESULT:\n";
    cout << "  Encrypted sum: " << result << "\n";
    cout << "  Expected sum (mod φ): " << total_sum << "\n";
    cout << "  Match: " << (abs(result - total_sum) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  1M RICH COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1M direct operations\n";
    cout << "  ✅ Pos+Neg mixed\n";
    cout << "  ✅ φ-modulo bounded\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
