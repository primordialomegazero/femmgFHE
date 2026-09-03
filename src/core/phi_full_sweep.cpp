// ============================================
// φ-FULL SWEEP — ALL COMBINATIONS
//
// Depth: 1, 2, 3, 5, 10
// Encoding: φ×10, φ×100, φ×1000, φ×10000
// Scaling: 40, 50, 59, 80
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

struct TestResult {
    int depth;
    int scale;
    int single_match;
    int chain_match;
    double time_ms;
};

int main() {
    cout << "========================================\n";
    cout << "  φ-FULL SWEEP\n";
    cout << "========================================\n\n";

    vector<int> depths = {1, 2, 3, 5, 10};
    vector<int> scales = {100, 1000, 10000};
    vector<int> modsizes = {40, 50, 59};

    vector<TestResult> results;

    for (int depth : depths) {
        for (int scale : scales) {
            for (int modsize : modsizes) {
                cout << "  Testing: Depth " << depth 
                     << ", Scale " << scale 
                     << ", ModSize " << modsize << "...\n";

                try {
                    CCParams<CryptoContextCKKSRNS> parameters;
                    parameters.SetMultiplicativeDepth(depth);
                    parameters.SetScalingModSize(modsize);
                    parameters.SetBatchSize(16);
                    parameters.SetSecurityLevel(HEStd_128_classic);

                    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
                    cc->Enable(PKE);
                    cc->Enable(KEYSWITCH);
                    cc->Enable(LEVELEDSHE);

                    auto keyPair = cc->KeyGen();

                    int PHI_INT = (int)round(1.6180339887498948482 * scale);
                    int PHI_INV_INT = (int)round(0.6180339887498948482 * scale);

                    auto encrypt_int = [&](int bit) {
                        int val = (bit == 0) ? PHI_INT : PHI_INV_INT;
                        vector<double> v(16, (double)val);
                        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                        return cc->Encrypt(keyPair.publicKey, pt);
                    };

                    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
                        Plaintext result_pt;
                        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                        result_pt->SetLength(16);
                        double sum = 0.0;
                        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                        return sum / 16.0;
                    };

                    auto exact_decode = [&](double val) {
                        int int_val = (int)round(val / scale);
                        int mod_phi = int_val % PHI_INT;
                        if (mod_phi < 0) mod_phi += PHI_INT;
                        
                        int zero_threshold = scale / 10;
                        if (mod_phi < zero_threshold || mod_phi > PHI_INT - zero_threshold) return 0;
                        
                        return (mod_phi < PHI_INT / 2) ? 1 : 0;
                    };

                    // Single additions
                    int single_match = 0;
                    for (int A : {0, 1}) {
                        for (int B : {0, 1}) {
                            for (int Cin : {0, 1}) {
                                auto ct_a = encrypt_int(A);
                                auto ct_b = encrypt_int(B);
                                auto ct_cin = encrypt_int(Cin);
                                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                                double avg = decrypt_avg(ct_sum);
                                int decoded = exact_decode(avg);
                                int expected = (A + B + Cin) % 2;
                                single_match += (decoded == expected);
                            }
                        }
                    }

                    // Chained
                    auto ct_acc = encrypt_int(0);
                    auto ct_add = encrypt_int(1);
                    int chain_match = 0;
                    auto start = high_resolution_clock::now();

                    for (int i = 0; i < 100; i++) {
                        ct_acc = cc->EvalAdd(ct_acc, ct_add);
                        double avg = decrypt_avg(ct_acc);
                        int decoded = exact_decode(avg);
                        int expected = (i + 1) % 2;
                        chain_match += (decoded == expected);
                    }

                    auto end = high_resolution_clock::now();
                    double time_ms = duration_cast<milliseconds>(end - start).count();

                    results.push_back({depth, scale, single_match, chain_match, time_ms});
                    cout << "  ✅ Depth " << depth << ", Scale " << scale 
                         << ", ModSize " << modsize << " → Single: " << single_match 
                         << "/8, Chain: " << chain_match << "/100\n";
                }
                catch (...) {
                    cout << "  ❌ Failed: Depth " << depth << ", Scale " << scale 
                         << ", ModSize " << modsize << "\n";
                }
            }
        }
    }

    // ============================================
    // BEST RESULTS
    // ============================================

    cout << "\n========================================\n";
    cout << "  TOP 10 RESULTS\n";
    cout << "========================================\n\n";

    sort(results.begin(), results.end(), [](const TestResult& a, const TestResult& b) {
        if (a.chain_match != b.chain_match) return a.chain_match > b.chain_match;
        return a.single_match > b.single_match;
    });

    cout << "  Rank | Depth | Scale | Single | Chain | Time\n";
    cout << "  -----|-------|-------|--------|-------|------\n";

    for (int i = 0; i < min(10, (int)results.size()); i++) {
        cout << "  " << setw(3) << i + 1 << " | "
             << setw(5) << results[i].depth << " | "
             << setw(5) << results[i].scale << " | "
             << setw(6) << results[i].single_match << "/8 | "
             << setw(5) << results[i].chain_match << "/100 | "
             << setw(5) << fixed << setprecision(0) << results[i].time_ms << "ms\n";
    }

    cout << "\n========================================\n";
    cout << "  FULL SWEEP COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
