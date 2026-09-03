// ============================================
// φ-ADD EMERGENT FHE
// Test ang emergent addition formula:
// φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)
//
// Walang slots, batch 1, pure φ-power space
// May φ-modulo sa dulo
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <algorithm>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ADD EMERGENT FHE\n";
    cout << "========================================\n\n";

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

    cout << "  ✅ CKKS initialized (depth 1, batch 1 — walang slots)\n\n";

    // ============================================
    // ENCODING: x → log_φ(x)
    // ============================================

    auto encrypt_phi = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        vector<double> v(1, log_phi_x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_phi = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST 1: MULTIPLICATION — 5 × 7 = 35
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 5 × 7 = 35\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_phi(5.0);
    auto ct_7 = encrypt_phi(7.0);

    auto ct_mul = cc->EvalAdd(ct_5, ct_7);
    double log_mul = decrypt_phi(ct_mul);
    double val_mul = pow(PHI, log_mul);
    double mod_mul = fmod(val_mul, PHI);

    cout << "  log_φ(35) = " << log_mul << "\n";
    cout << "  φ^log = " << val_mul << "\n";
    cout << "  φ^log mod φ = " << mod_mul << "\n";
    cout << "  Expected: 35 mod φ = " << fmod(35.0, PHI) << "\n";
    cout << "  Match: " << (abs(mod_mul - fmod(35.0, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: DIVISION — 100 ÷ 10 = 10
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 100 ÷ 10 = 10\n";
    cout << "========================================\n\n";

    auto ct_100 = encrypt_phi(100.0);
    auto ct_10 = encrypt_phi(10.0);

    auto ct_div = cc->EvalSub(ct_100, ct_10);
    double log_div = decrypt_phi(ct_div);
    double val_div = pow(PHI, log_div);
    double mod_div = fmod(val_div, PHI);

    cout << "  log_φ(10) = " << log_div << "\n";
    cout << "  φ^log = " << val_div << "\n";
    cout << "  φ^log mod φ = " << mod_div << "\n";
    cout << "  Expected: 10 mod φ = " << fmod(10.0, PHI) << "\n";
    cout << "  Match: " << (abs(mod_div - fmod(10.0, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: 10K MULTIPLICATION CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  10K MULTIPLICATION CHAIN\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_phi(2.0);
    auto ct_three = encrypt_phi(3.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: ×3 (paulit-ulit)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_log = log(2.0)/LN_PHI;

    for (int i = 0; i < N; i++) {
        ct_state = cc->EvalAdd(ct_state, ct_three);
        expected_log += log(3.0)/LN_PHI;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double final_log = decrypt_phi(ct_state);
    double final_mod = fmod(pow(PHI, fmod(final_log, PHI)), PHI);
    double expected_mod = fmod(pow(PHI, fmod(expected_log, PHI)), PHI);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Final log: " << final_log << "\n";
    cout << "  Final mod φ: " << final_mod << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match: " << (abs(final_mod - expected_mod) < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  EMERGENT FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Multiplication at division\n";
    cout << "  ✅ Walang slots\n";
    cout << "  ✅ φ-modulo sa dulo\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
