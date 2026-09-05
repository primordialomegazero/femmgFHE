// ============================================
// φ-CHAIN DEBUG — Quick test na may print
// Tingnan ang bawat step ng chain
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

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

    // Exact φ-gap decomposition
    auto exact_decompose = [&](int x) {
        vector<pair<int,int>> gaps;
        double remaining = x;
        
        for (int iter = 0; iter < 50 && abs(remaining) > 1e-10; iter++) {
            bool found = false;
            
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
    cout << "  φ-CHAIN DEBUG — Quick test\n";
    cout << "========================================\n\n";

    // Start sa φ⁴
    auto ct = encrypt_log(4.0);
    double n_start = 4.0;
    
    cout << "  Start: n=" << n_start << ", value=" << pow(PHI, 4) << "\n\n";

    // Operations: +5, ×3, -2, ÷2, +10
    vector<int> adds = {5, -2, 10};
    vector<int> muls = {3};
    vector<int> divs = {2};

    double current_n = n_start;
    double expected_n = n_start;

    // +5
    auto gaps_5 = exact_decompose(5);
    double corr_5 = total_correction(gaps_5);
    vector<double> d5(1, abs(corr_5));
    Plaintext pt5 = cc->MakeCKKSPackedPlaintext(d5);
    ct = cc->EvalAdd(ct, pt5);
    current_n = fmod(decrypt_log(ct), PHI);
    expected_n = fmod(expected_n + corr_5, PHI);
    
    cout << "  +5 → corr=" << corr_5 << ", n=" << current_n 
         << ", expected=" << expected_n 
         << ", diff=" << current_n - expected_n << "\n";

    // ×3
    double corr_mul3 = log(3.0) / LN_PHI;
    vector<double> dm3(1, corr_mul3);
    Plaintext ptm3 = cc->MakeCKKSPackedPlaintext(dm3);
    ct = cc->EvalAdd(ct, ptm3);
    current_n = fmod(decrypt_log(ct), PHI);
    expected_n = fmod(expected_n + corr_mul3, PHI);
    
    cout << "  ×3 → corr=" << corr_mul3 << ", n=" << current_n 
         << ", expected=" << expected_n 
         << ", diff=" << current_n - expected_n << "\n";

    // -2
    auto gaps_2 = exact_decompose(2);
    double corr_2 = -total_correction(gaps_2);
    vector<double> d2(1, abs(corr_2));
    Plaintext pt2 = cc->MakeCKKSPackedPlaintext(d2);
    ct = cc->EvalSub(ct, pt2);
    current_n = fmod(decrypt_log(ct), PHI);
    expected_n = fmod(expected_n + corr_2, PHI);
    
    cout << "  -2 → corr=" << corr_2 << ", n=" << current_n 
         << ", expected=" << expected_n 
         << ", diff=" << current_n - expected_n << "\n";

    // ÷2
    double corr_div2 = -log(2.0) / LN_PHI;
    vector<double> dd2(1, abs(corr_div2));
    Plaintext ptd2 = cc->MakeCKKSPackedPlaintext(dd2);
    ct = cc->EvalSub(ct, ptd2);
    current_n = fmod(decrypt_log(ct), PHI);
    expected_n = fmod(expected_n + corr_div2, PHI);
    
    cout << "  ÷2 → corr=" << corr_div2 << ", n=" << current_n 
         << ", expected=" << expected_n 
         << ", diff=" << current_n - expected_n << "\n";

    // +10
    auto gaps_10 = exact_decompose(10);
    double corr_10 = total_correction(gaps_10);
    vector<double> d10(1, abs(corr_10));
    Plaintext pt10 = cc->MakeCKKSPackedPlaintext(d10);
    ct = cc->EvalAdd(ct, pt10);
    current_n = fmod(decrypt_log(ct), PHI);
    expected_n = fmod(expected_n + corr_10, PHI);
    
    cout << "  +10 → corr=" << corr_10 << ", n=" << current_n 
         << ", expected=" << expected_n 
         << ", diff=" << current_n - expected_n << "\n\n";

    cout << "  Final n: " << current_n << "\n";
    cout << "  Expected: " << expected_n << "\n";
    cout << "  Total diff: " << current_n - expected_n << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
