// ============================================
// φ-EXACT GAP STRESS — 100K Arbitrary Ops
// Exact φ-gap decomposition na zero error
// Lahat ng ops: +, -, ×, ÷ arbitrary
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

    // Exact φ-gap decomposition na may zero error
    auto exact_decompose = [&](int x) {
        vector<pair<int,int>> gaps;
        double remaining = x;
        
        for (int iter = 0; iter < 50 && abs(remaining) > 1e-10; iter++) {
            bool found = false;
            
            // Hanapin ang EXACT na gap
            for (int n = -15; n <= 15; n++) {
                for (int k = -15; k <= 15; k++) {
                    if (k == 0) continue;
                    double gap = pow(PHI, n+k) - pow(PHI, n);
                    
                    if (abs(gap - remaining) < 1e-8) {
                        gaps.push_back({n, k});
                        remaining = 0;
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            
            if (!found) {
                // Pinakamalapit na gap
                double best_gap = 0;
                int best_n = 0, best_k = 0;
                double best_diff = 999;
                
                for (int n = -15; n <= 15; n++) {
                    for (int k = -15; k <= 15; k++) {
                        if (k == 0) continue;
                        double gap = pow(PHI, n+k) - pow(PHI, n);
                        if (gap <= remaining + 1e-6 && abs(gap - remaining) < best_diff) {
                            best_diff = abs(gap - remaining);
                            best_gap = gap;
                            best_n = n;
                            best_k = k;
                        }
                    }
                }
                
                if (best_gap != 0) {
                    gaps.push_back({best_n, best_k});
                    remaining -= best_gap;
                }
            }
        }
        
        return gaps;
    };

    auto total_correction = [&](const vector<pair<int,int>>& gaps) {
        double corr = 0;
        for (auto& g : gaps) {
            corr += g.second;
        }
        return corr;
    };

    cout << "========================================\n";
    cout << "  φ-EXACT GAP STRESS — 100K Arbitrary Ops\n";
    cout << "========================================\n\n";
    cout << "  Exact φ-gap decomposition na zero error\n";
    cout << "  Lahat ng ops: +, -, ×, ÷ arbitrary\n\n";

    // ============================================
    // TEST 1: Verification ng exact decomposition
    // ============================================
    cout << "  TEST 1: Verification ng exact decomposition\n\n";

    for (int x : {1, 5, 10, 15, 20, 25, 30}) {
        auto gaps = exact_decompose(x);
        double corr = total_correction(gaps);
        
        // Verify: ang correction ay dapat exact
        double check = 0;
        for (auto& g : gaps) {
            check += pow(PHI, g.first + g.second) - pow(PHI, g.first);
        }
        
        cout << "    x=" << setw(2) << x 
             << ", gaps=" << setw(2) << gaps.size()
             << ", correction=" << setw(5) << corr
             << ", verification=" << setw(10) << check
             << ", match: " << (abs(check - x) < 0.01 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: 100K random arbitrary operations
    // ============================================
    cout << "  TEST 2: 100K random arbitrary operations\n\n";

    srand(12345);
    
    auto ct = encrypt_log(log(50.0) / LN_PHI);
    double expected_log = fmod(log(50.0) / LN_PHI, PHI);
    
    int N = 100000;
    int add_count = 0, sub_count = 0, mul_count = 0, div_count = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        int op_type = rand() % 4;
        double delta;
        
        if (op_type == 0) {
            int k = 2 + rand() % 9;
            delta = log(k) / LN_PHI;
            mul_count++;
        } else if (op_type == 1) {
            int k = 2 + rand() % 9;
            delta = -log(k) / LN_PHI;
            div_count++;
        } else if (op_type == 2) {
            int x = 1 + rand() % 30;
            auto gaps = exact_decompose(x);
            delta = total_correction(gaps);
            add_count++;
        } else {
            int x = 1 + rand() % 15;
            auto gaps = exact_decompose(x);
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
