// ============================================
// φ-CT×CT — Ciphertext Multiplication sa Log Space
// ct_a × ct_b = ct_a + ct_b (log space)
// Walang EvalMult, walang depth reduction
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
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

    auto encrypt_val = [&](double val) {
        // I-encode bilang log_φ(val)
        double log_val = log(val) / LN_PHI;
        vector<double> v(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        double log_val = pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);  // recover normal value
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-CT×CT — Ciphertext Multiplication\n";
    cout << "========================================\n\n";
    cout << "  ct_a × ct_b = EvalAdd(ct_a, ct_b)\n";
    cout << "  Walang EvalMult, walang depth reduction\n\n";

    // ============================================
    // TEST 1: 7 × 13 = 91
    // ============================================
    cout << "  TEST 1: 7 × 13 = 91\n\n";

    auto ct_7 = encrypt_val(7.0);
    auto ct_13 = encrypt_val(13.0);

    cout << "    ct_7 log: " << decrypt_log(ct_7) << "\n";
    cout << "    ct_13 log: " << decrypt_log(ct_13) << "\n";

    // CT × CT = EvalAdd
    auto ct_product = cc->EvalAdd(ct_7, ct_13);

    double val_product = decrypt_val(ct_product);
    double log_product = decrypt_log(ct_product);

    cout << "    After EvalAdd:\n";
    cout << "    Log: " << log_product << "\n";
    cout << "    Value: " << val_product << "\n";
    cout << "    Expected: 91\n";
    cout << "    Match: " << (abs(val_product - 91.0) < 0.01 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct_product->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Chain ng CT × CT
    // ============================================
    cout << "  TEST 2: Chain ng CT × CT\n\n";

    auto ct_acc = encrypt_val(2.0);  // Start sa 2
    
    cout << "    Start: 2\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10; i++) {
        auto ct_next = encrypt_val(3.0 + i);  // ×3, ×4, ×5, ...
        ct_acc = cc->EvalAdd(ct_acc, ct_next);  // CT × CT
        cout << "    ×" << (3+i) << " → " << decrypt_val(ct_acc) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 10K CT × CT operations
    // ============================================
    cout << "  TEST 3: 10K CT × CT operations\n\n";

    ct_acc = encrypt_val(1.5);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        auto ct_factor = encrypt_val(1.001);  // ×1.001
        ct_acc = cc->EvalAdd(ct_acc, ct_factor);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    double val_10k = decrypt_val(ct_acc);
    
    cout << "    10K × 1.001:\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Final value: " << val_10k << "\n";
    cout << "    Expected: " << 1.5 * pow(1.001, 10000) << "\n";
    cout << "    Level: " << ct_acc->GetLevel() << "\n";

    return 0;
}
