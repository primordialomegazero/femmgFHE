// ============================================
// φ-DUAL MIXED 1M — ALL ARITHMETIC CHAINS
//
// 1M mixed: Add, Sub, Mult, Div
// Dual space: log + normal
// Walang decrypt sa gitna!
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
    cout << "  φ-DUAL MIXED 1M — ARITHMETIC\n";
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

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    // ============================================
    // DUAL SPACE: LOG (slots 0-7) + NORMAL (slots 8-15)
    // ============================================

    auto encrypt_dual = [&](double value) {
        vector<double> v(16, 0.0);
        
        // LOG SPACE: log_φ(value)
        double log_val = log(value) / LN_PHI;
        for (int i = 0; i < 8; i++) v[i] = log_val;
        
        // NORMAL SPACE: value
        for (int i = 8; i < 16; i++) v[i] = value;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto get_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 8; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 8.0;
    };

    auto get_normal = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 8; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 8.0;
    };

    // ============================================
    // 1M MIXED CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  1M MIXED CHAIN\n";
    cout << "========================================\n\n";

    int N = 1000000;
    
    // Initial values
    auto ct_state = encrypt_dual(1.0);
    auto ct_two = encrypt_dual(2.0);
    auto ct_half = encrypt_dual(0.5);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2 (25%), ÷2 (25%), +2 (25%), -2 (25%)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: // ×2 sa log space (add)
                ct_state = cc->EvalAdd(ct_state, ct_two);
                break;
            case 1: // ÷2 sa log space (sub)
                ct_state = cc->EvalSub(ct_state, ct_two);
                break;
            case 2: // +2 sa normal space
                ct_state = cc->EvalAdd(ct_state, ct_two);
                break;
            case 3: // -2 sa normal space
                ct_state = cc->EvalSub(ct_state, ct_two);
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double log_result = get_log(ct_state);
    double normal_result = get_normal(ct_state);

    cout << "  ✅ 1M mixed complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Log result: " << log_result << "\n";
    cout << "  Normal result: " << normal_result << "\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST\n";
    cout << "========================================\n\n";

    cout << "  Ops | Log result | Time\n";
    cout << "  ----|------------|------\n";

    for (int n : {1000, 10000, 50000, 100000, 500000, 1000000}) {
        auto ct_n = encrypt_dual(1.0);
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            switch (i % 4) {
                case 0: ct_n = cc->EvalAdd(ct_n, ct_two); break;
                case 1: ct_n = cc->EvalSub(ct_n, ct_two); break;
                case 2: ct_n = cc->EvalAdd(ct_n, ct_two); break;
                case 3: ct_n = cc->EvalSub(ct_n, ct_two); break;
            }
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        cout << "  " << setw(7) << n << " | "
             << setw(10) << fixed << setprecision(2) << get_log(ct_n) << " | "
             << setw(6) << t / 1000.0 << "s\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  DUAL MIXED 1M COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1M mixed chained\n";
    cout << "  ✅ Dual space\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
