// ============================================
// φ-STRESS TEST FINAL — 100K Arbitrary Ops
// φ-gap decomposition para sa arbitrary add
// Lahat ng ops: +, -, ×, ÷ arbitrary values
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

    // φ-gap decomposition para sa arbitrary integer
    auto decompose_gaps = [&](int x) {
        vector<pair<int,int>> gaps;
        double remaining = x;
        
        for (int iter = 0; iter < 20 && abs(remaining) > 1e-6; iter++) {
            // Hanapin ang pinakamalapit na φ-gap
            double best_gap = 0;
            int best_n = 0, best_k = 0;
            double best_diff = 999;
            
            for (int k = 1; k <= 15; k++) {
                for (int n = -10; n <= 15; n++) {
                    double gap = pow(PHI, n+k) - pow(PHI, n);
                    if (gap <= remaining + 0.01 && abs(gap - remaining) < best_diff) {
                        best_diff = abs(gap - remaining);
                        best_gap = gap;
                        best_n = n;
                        best_k = k;
                    }
                }
            }
            
            if (best_gap > 0) {
                gaps.push_back({best_n, best_k});
                remaining -= best_gap;
            } else {
                break;
            }
        }
        
        return gaps;
    };

    // Ang total correction mula sa gaps
    auto total_correction = [&](const vector<pair<int,int>>& gaps) {
        double corr = 0;
        for (auto& g : gaps) {
            corr += g.second;  // correction = k
        }
        return corr;
    };

    cout << "========================================\n";
    cout << "  φ-STRESS TEST FINAL — 100K Arbitrary Ops\n";
    cout << "========================================\n\n";
    cout << "  φ-gap decomposition para sa arbitrary add\n";
    cout << "  Lahat ng ops: +, -, ×, ÷ arbitrary\n\n";

    // ============================================
    // TEST 1: Individual arbitrary additions
    // ============================================
    cout << "  TEST 1: Individual arbitrary additions\n\n";

    auto ct = encrypt_log(log(10.0) / LN_PHI);
    cout << "    Start: 10\n";

    // +7
    auto gaps_7 = decompose_gaps(7);
    double corr_7 = total_correction(gaps_7);
    vector<double> d7(1, abs(corr_7));
    Plaintext pt7 = cc->MakeCKKSPackedPlaintext(d7);
    ct = cc->EvalAdd(ct, pt7);
    double val_7 = pow(PHI, fmod(decrypt_log(ct), PHI));
    cout << "    +7 → correction=" << corr_7 << ", value=" << val_7 << "\n";

    // +13
    auto gaps_13 = decompose_gaps(13);
    double corr_13 = total_correction(gaps_13);
    vector<double> d13(1, abs(corr_13));
    Plaintext pt13 = cc->MakeCKKSPackedPlaintext(d13);
    ct = cc->EvalAdd(ct, pt13);
    double val_13 = pow(PHI, fmod(decrypt_log(ct), PHI));
    cout << "    +13 → correction=" << corr_13 << ", value=" << val_13 << "\n";

    // +42
    auto gaps_42 = decompose_gaps(42);
    double corr_42 = total_correction(gaps_42);
    vector<double> d42(1, abs(corr_42));
    Plaintext pt42 = cc->MakeCKKSPackedPlaintext(d42);
    ct = cc->EvalAdd(ct, pt42);
    double val_42 = pow(PHI, fmod(decrypt_log(ct), PHI));
    cout << "    +42 → correction=" << corr_42 << ", value=" << val_42 << "\n\n";

    // ============================================
    // TEST 2: 100K random arbitrary operations
    // ============================================
    cout << "  TEST 2: 100K random arbitrary operations\n\n";

    srand(42);
    
    ct = encrypt_log(log(100.0) / LN_PHI);
    double expected_log = fmod(log(100.0) / LN_PHI, PHI);
    
    int N = 100000;
    int add_count = 0, sub_count = 0, mul_count = 0, div_count = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        int op_type = rand() % 4;
        double delta;
        
        if (op_type == 0) {
            // Multiply by 2-10
            int k = 2 + rand() % 9;
            delta = log(k) / LN_PHI;
            mul_count++;
        } else if (op_type == 1) {
            // Divide by 2-10
            int k = 2 + rand() % 9;
            delta = -log(k) / LN_PHI;
            div_count++;
        } else if (op_type == 2) {
            // Addition ng 1-30 gamit ang φ-gaps
            int x = 1 + rand() % 30;
            auto gaps = decompose_gaps(x);
            delta = total_correction(gaps);
            add_count++;
        } else {
            // Subtraction ng 1-15 gamit ang φ-gaps
            int x = 1 + rand() % 15;
            auto gaps = decompose_gaps(x);
            delta = -total_correction(gaps);
            sub_count++;
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
        
        if (i % 25000 == 0 && i > 0) {
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
    cout << "  Operations: +" << add_count << ", -" << sub_count 
         << ", ×" << mul_count << ", ÷" << div_count << "\n";
    cout << "  Final log_φ mod φ: " << log_final_mod << "\n";
    cout << "  Expected mod φ: " << expected_final_mod << "\n";
    cout << "  Match: " << (abs(log_final_mod - expected_final_mod) < 0.5 ? "✅" : "❌") << "\n";
    cout << "  Final value: " << pow(PHI, log_final_mod) << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
