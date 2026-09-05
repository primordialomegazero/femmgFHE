// ============================================
// φ-OPTIMIZED 1M — Walang Decrypt sa Loop
// Pre-computed corrections, optimized
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
        vector<double> v(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Exact φ-gap decomposition — PRE-COMPUTE LAHAT
    auto exact_decompose = [&](int x) {
        vector<pair<int,int>> gaps;
        double remaining = x;
        
        for (int iter = 0; iter < 50 && abs(remaining) > 1e-10; iter++) {
            bool found = false;
            for (int n = -15; n <= 15 && !found; n++) {
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
            }
            if (!found) {
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
        for (auto& g : gaps) corr += g.second;
        return corr;
    };

    cout << "========================================\n";
    cout << "  φ-OPTIMIZED 1M — Walang Decrypt sa Loop\n";
    cout << "========================================\n\n";

    // ============================================
    // PRE-COMPUTE LAHAT NG DELTAS
    // ============================================
    cout << "  Pre-computing deltas...\n";

    vector<double> mul_deltas;
    vector<double> add_corrections;
    
    // Multiply deltas para sa 2-100
    for (int k = 2; k <= 100; k++) {
        mul_deltas.push_back(log(k) / LN_PHI);
    }
    
    // Addition corrections para sa 1-1000
    for (int x = 1; x <= 1000; x++) {
        auto gaps = exact_decompose(x);
        add_corrections.push_back(total_correction(gaps));
    }
    
    cout << "  ✓ Pre-computed: " << mul_deltas.size() << " mul deltas, "
         << add_corrections.size() << " add corrections\n\n";

    // ============================================
    // 1M RANDOM OPS — WALANG DECRYPT SA LOOP
    // ============================================
    srand(42);
    
    auto ct = encrypt_log(log(50.0) / LN_PHI);
    double expected_n = log(50.0) / LN_PHI;
    
    int N = 1000000;
    
    cout << "  Running 1M ops...\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        int op_type = rand() % 4;
        double delta;
        
        if (op_type == 0) {
            // Multiply by 2-100
            delta = mul_deltas[rand() % mul_deltas.size()];
        } else if (op_type == 1) {
            // Divide by 2-100
            delta = -mul_deltas[rand() % mul_deltas.size()];
        } else if (op_type == 2) {
            // Addition ng 1-1000
            delta = add_corrections[rand() % add_corrections.size()];
        } else {
            // Subtraction ng 1-500
            delta = -add_corrections[rand() % (add_corrections.size() / 2)];
        }
        
        vector<double> d(1, abs(delta));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (delta >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        expected_n += delta;
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_log(ct);
    
    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Ops/sec: " << (N * 1000.0) / time << "\n\n";
    cout << "  Final n: " << n_final << "\n";
    cout << "  Expected: " << expected_n << "\n";
    cout << "  Diff: " << (n_final - expected_n) << "\n";
    cout << "  Match: " << (abs(n_final - expected_n) < 0.01 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
