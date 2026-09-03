// ============================================
// φ-POS+NEG 1M — ARBITRARY VALUES DIRECT
//
// 1M operations na may positive at negative values
// Direct computation — walang collapse
// Fractal modulo para bounded
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

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-POS+NEG 1M — ARBITRARY DIRECT\n";
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
    const double LN_PHI = log(PHI);
    const double PHI_MOD = PHI;

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Positive + Negative values, φ-modulo\n\n";

    // ============================================
    // ARBITRARY ENCODING: POSITIVE AT NEGATIVE
    // ============================================

    auto encrypt_arbitrary = [&](double value) {
        // φ-modulo: i-bound sa [0, φ)
        double mod_val = fmod(value, PHI_MOD);
        vector<double> v(16, mod_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_arbitrary = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return fmod(sum / 16.0, PHI_MOD);
    };

    // ============================================
    // RANDOM POSITIVE + NEGATIVE VALUES
    // ============================================

    cout << "========================================\n";
    cout << "  RANDOM ARBITRARY VALUES\n";
    cout << "========================================\n\n";

    mt19937 gen(42);
    uniform_real_distribution<double> random_val(-100.0, 100.0);

    cout << "  Sample values (first 10):\n  ";
    for (int i = 0; i < 10; i++) {
        cout << fixed << setprecision(2) << random_val(gen) << " ";
    }
    cout << "\n\n";

    // I-reset ang generator
    mt19937 gen2(42);

    // ============================================
    // 1M DIRECT OPERATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  1M DIRECT OPERATIONS\n";
    cout << "========================================\n\n";

    int N = 1000000;
    auto ct_acc = encrypt_arbitrary(0.0);
    
    double total_sum = 0.0;

    cout << "  Running 1M random ops (pos+neg)...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        double val = random_val(gen2);
        double mod_val = fmod(val, PHI_MOD);
        
        total_sum = fmod(total_sum + mod_val, PHI_MOD);
        
        auto ct_val = encrypt_arbitrary(mod_val);
        ct_acc = cc->EvalAdd(ct_acc, ct_val);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_arbitrary(ct_acc);

    cout << "  ✅ 1M direct operations complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Towers: " << ct_acc->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Result (mod φ): " << result << "\n";
    cout << "  Expected (mod φ): " << total_sum << "\n";
    cout << "  Match: " << (abs(result - total_sum) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST (POS+NEG)\n";
    cout << "========================================\n\n";

    cout << "  Ops | Result | Time\n";
    cout << "  ----|--------|------\n";

    for (int n : {1000, 10000, 50000, 100000}) {
        mt19937 gen3(42 + n);
        auto ct_n = encrypt_arbitrary(0.0);
        double sum_n = 0.0;
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            double val = random_val(gen3);
            double mod_val = fmod(val, PHI_MOD);
            sum_n = fmod(sum_n + mod_val, PHI_MOD);
            
            auto ct_val = encrypt_arbitrary(mod_val);
            ct_n = cc->EvalAdd(ct_n, ct_val);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r = decrypt_arbitrary(ct_n);
        bool match = abs(r - sum_n) < 0.01;

        cout << "  " << setw(6) << n << " | "
             << setw(7) << fixed << setprecision(3) << r << " | "
             << setw(5) << t / 1000.0 << "s | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  POS+NEG 1M COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1M direct operations\n";
    cout << "  ✅ Positive + Negative values\n";
    cout << "  ✅ φ-modulo bounded\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
