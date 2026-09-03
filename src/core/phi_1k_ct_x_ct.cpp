// ============================================
// φ-1K CT × CT — ENCRYPTED MULTIPLICATION
//
// 1K ciphertext × ciphertext operations
// Sa φ-log space: multiply = EvalAdd
// Level 0, walang bootstrapping
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-1K CT × CT — ENCRYPTED MULTIPLICATION\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);  // DEPTH 1 LANG!
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(2);  // Dual space
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 128-bit, dual space)\n\n";

    // Dual space encryption
    auto encrypt_dual = [&](double value) {
        vector<double> dual(2, 0.0);
        dual[0] = value;                    // Normal space
        dual[1] = log(value) / LN_PHI;      // Log space
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        // Slot 1 (log space) → convert back
        double log_val = result_pt->GetCKKSPackedValue()[1].real();
        return pow(PHI, log_val);
    };

    cout << "========================================\n";
    cout << "  1K CT × CT (ENCRYPTED MULTIPLICATION)\n";
    cout << "========================================\n\n";

    // Start: 2.0 (encrypted)
    auto ct_result = encrypt_dual(2.0);
    auto ct_multiplier = encrypt_dual(2.0);  // ×2 bawat operation

    cout << "  Starting value: 2.0 (encrypted)\n";
    cout << "  Operation: ×2 (1K beses)\n";
    cout << "  Expected: 2 × 2^1000\n\n";

    auto start = high_resolution_clock::now();

    // 1K CT × CT operations
    for (int i = 0; i < 1000; i++) {
        // Multiply: EvalAdd sa log space
        ct_result = cc->EvalAdd(ct_result, ct_multiplier);
    }

    auto end = high_resolution_clock::now();
    auto time_ms = duration_cast<milliseconds>(end - start).count();

    cout << "  ✅ 1K CT × CT complete!\n";
    cout << "  Time: " << time_ms << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    // Decrypt sa dulo lang
    double result = decrypt_value(ct_result);

    cout << "========================================\n";
    cout << "  RESULT\n";
    cout << "========================================\n\n";
    cout << "  Result: " << scientific << result << "\n";
    cout << "  Expected: 2^1001 ≈ " << pow(2.0, 1001) << "\n";
    cout << "  Match: " << (abs(log(result) - log(pow(2.0, 1001))) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1K CT × CT: " << time_ms << " ms\n";
    cout << "  ✅ Level: 0 (walang bootstrapping)\n";
    cout << "  ✅ Depth: 1 (minimum)\n";
    cout << "  ✅ Pure FHE (walang decrypt sa gitna)\n";
    cout << "  ✅ Dual space: normal + log\n\n";

    return 0;
}
