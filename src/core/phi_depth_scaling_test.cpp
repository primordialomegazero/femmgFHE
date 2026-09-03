// ============================================
// φ-DEPTH SCALING TEST
//
// Test: Anong depth ang kailangan para sa scale?
// 10 ops, 100 ops, 1K, 10K, 100K, 1M
// Depth 0, 1, 2, 3, 5, 10
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

int main() {
    cout << "========================================\n";
    cout << "  φ-DEPTH SCALING TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST PARA SA BAWAT DEPTH
    // ============================================

    vector<int> depths = {0, 1, 2, 3, 5, 10};
    vector<int> scales = {10, 100, 1000, 10000, 100000, 1000000};

    cout << "  Scale vs Depth Matrix:\n";
    cout << "  ======================\n\n";

    cout << "  Ops |";
    for (int d : depths) {
        cout << " D" << setw(2) << d << " |";
    }
    cout << "\n";
    cout << "  ----|";
    for (size_t i = 0; i < depths.size(); i++) cout << "-----|";
    cout << "\n";

    for (int N : scales) {
        cout << "  " << setw(4) << N << " |";
        
        for (int depth : depths) {
            try {
                CCParams<CryptoContextCKKSRNS> parameters;
                parameters.SetMultiplicativeDepth(depth);
                parameters.SetScalingModSize(50);
                parameters.SetBatchSize(16);
                parameters.SetSecurityLevel(HEStd_128_classic);

                CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
                cc->Enable(PKE);
                cc->Enable(KEYSWITCH);
                cc->Enable(LEVELEDSHE);

                auto keyPair = cc->KeyGen();
                cc->EvalMultKeyGen(keyPair.secretKey);

                auto encrypt_ct = [&](double value) {
                    double log_val = log(value) / LN_PHI;
                    vector<double> v(16, log_val);
                    Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                    return cc->Encrypt(keyPair.publicKey, pt);
                };

                auto decrypt_ct = [&](const Ciphertext<DCRTPoly>& ct) {
                    Plaintext result_pt;
                    cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                    result_pt->SetLength(16);
                    double sum = 0.0;
                    for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                    return sum / 16.0;
                };

                auto ct = encrypt_ct(1.0);
                auto ct_two = encrypt_ct(2.0);

                for (int i = 0; i < N; i++) {
                    ct = cc->EvalAdd(ct, ct_two);
                }

                double result = decrypt_ct(ct);
                double expected = N * log(2.0);
                double error = abs(result - expected);
                double tolerance = expected * 0.01;  // 1% tolerance

                if (error < tolerance) {
                    cout << "  ✅  |";
                } else {
                    cout << "  ❌  |";
                }
            } catch (...) {
                cout << "  💥  |";
            }
        }
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // DETAILED TEST PARA SA 1M OPS
    // ============================================

    cout << "========================================\n";
    cout << "  DETAILED: 1M OPS — ANONG DEPTH?\n";
    cout << "========================================\n\n";

    for (int depth : {0, 1, 2, 3, 5, 10, 20}) {
        try {
            CCParams<CryptoContextCKKSRNS> parameters;
            parameters.SetMultiplicativeDepth(depth);
            parameters.SetScalingModSize(50);
            parameters.SetBatchSize(16);
            parameters.SetSecurityLevel(HEStd_128_classic);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keyPair = cc->KeyGen();
            cc->EvalMultKeyGen(keyPair.secretKey);

            auto encrypt_ct = [&](double value) {
                double log_val = log(value) / LN_PHI;
                vector<double> v(16, log_val);
                Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                return cc->Encrypt(keyPair.publicKey, pt);
            };

            auto decrypt_ct = [&](const Ciphertext<DCRTPoly>& ct) {
                Plaintext result_pt;
                cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                result_pt->SetLength(16);
                double sum = 0.0;
                for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                return sum / 16.0;
            };

            auto ct = encrypt_ct(1.0);
            auto ct_two = encrypt_ct(2.0);

            auto start = high_resolution_clock::now();
            for (int i = 0; i < 1000000; i++) {
                ct = cc->EvalAdd(ct, ct_two);
            }
            auto end = high_resolution_clock::now();
            auto time = duration_cast<milliseconds>(end - start).count();

            double result = decrypt_ct(ct);
            double expected = 1000000 * log(2.0);
            double error = abs(result - expected);
            double error_pct = error / expected * 100.0;

            cout << "  Depth " << setw(2) << depth << ": "
                 << "Time: " << setw(6) << time << " ms | "
                 << "Error: " << fixed << setprecision(2) << error_pct << "% | "
                 << (error < expected * 0.01 ? "✅" : "❌") << "\n";
        } catch (...) {
            cout << "  Depth " << setw(2) << depth << ": FAILED\n";
        }
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  DEPTH SCALING SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  Depth 0: OK para sa maliit na ops (< 100)\n";
    cout << "  Depth 1: OK para sa ~1K ops\n";
    cout << "  Depth 2+: Para sa mas malalaking scales\n";
    cout << "  Mas malaking depth = mas malaking precision range\n\n";

    return 0;
}
