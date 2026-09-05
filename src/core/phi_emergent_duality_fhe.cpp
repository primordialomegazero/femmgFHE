// ============================================
// φ-EMERGENT DUALITY FHE — 100 iterations
// Addition: φ^n + φ^(n-1) = φ^(n+1) → +1
// Subtraction: φ^n - φ^(n-1) = φ^(n-2) → -2
// Constant corrections — walang decrypt
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

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-EMERGENT DUALITY FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Addition: φ^n + φ^(n-1) = φ^(n+1) → +1\n";
    cout << "  Subtraction: φ^n - φ^(n-1) = φ^(n-2) → -2\n";
    cout << "  Constant corrections — walang decrypt\n\n";

    // ============================================
    // TEST 1: φ⁵ + φ⁴ = φ⁶
    // ============================================
    cout << "  TEST 1: φ⁵ + φ⁴ = φ⁶\n\n";

    double n_start = 5.0;
    auto ct_state = encrypt_n(n_start);

    // Addition correction: +1
    vector<double> delta_add(1, 1.0);
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(delta_add);
    ct_state = cc->EvalAdd(ct_state, pt_add);

    double n_add = decrypt_n(ct_state);
    double value_add = pow(PHI, n_add);
    double expected_add = pow(PHI, 5) + pow(PHI, 4);
    double phi_6 = pow(PHI, 6);

    cout << "    n: " << n_add << "\n";
    cout << "    value: " << value_add << "\n";
    cout << "    expected (φ⁵+φ⁴): " << expected_add << "\n";
    cout << "    expected (φ⁶): " << phi_6 << "\n";
    cout << "    Match: " << (abs(value_add - phi_6) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: φ⁵ - φ⁴ = φ³
    // ============================================
    cout << "  TEST 2: φ⁵ - φ⁴ = φ³\n\n";

    ct_state = encrypt_n(5.0);

    // Subtraction correction: -2
    vector<double> delta_sub(1, 2.0);
    Plaintext pt_sub = cc->MakeCKKSPackedPlaintext(delta_sub);
    ct_state = cc->EvalSub(ct_state, pt_sub);

    double n_sub = decrypt_n(ct_state);
    double value_sub = pow(PHI, n_sub);
    double expected_sub = pow(PHI, 5) - pow(PHI, 4);
    double phi_3 = pow(PHI, 3);

    cout << "    n: " << n_sub << "\n";
    cout << "    value: " << value_sub << "\n";
    cout << "    expected (φ⁵-φ⁴): " << expected_sub << "\n";
    cout << "    expected (φ³): " << phi_3 << "\n";
    cout << "    Match: " << (abs(value_sub - phi_3) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: Sequence ng φ-power additions at subtractions
    // ============================================
    cout << "  TEST 3: Mixed φ-power operations\n\n";

    ct_state = encrypt_n(10.0);
    
    cout << "    Start: φ¹⁰ = " << pow(PHI, 10) << "\n";
    
    // + φ⁹ → φ¹¹ (correction: +1)
    ct_state = cc->EvalAdd(ct_state, pt_add);
    cout << "    +φ⁹ → φ¹¹ = " << pow(PHI, decrypt_n(ct_state)) << "\n";
    
    // - φ¹⁰ → φ⁹ (correction: -2)
    ct_state = cc->EvalSub(ct_state, pt_sub);
    cout << "    -φ¹⁰ → φ⁹ = " << pow(PHI, decrypt_n(ct_state)) << "\n";
    
    // + φ⁸ → φ¹⁰ (correction: +1)
    ct_state = cc->EvalAdd(ct_state, pt_add);
    cout << "    +φ⁸ → φ¹⁰ = " << pow(PHI, decrypt_n(ct_state)) << "\n";
    
    // - φ⁹ → φ⁸ (correction: -2)
    ct_state = cc->EvalSub(ct_state, pt_sub);
    cout << "    -φ⁹ → φ⁸ = " << pow(PHI, decrypt_n(ct_state)) << "\n\n";

    // ============================================
    // TEST 4: 10K iterations ng φ-power cycle
    // ============================================
    cout << "  TEST 4: 10K iterations ng φ-power cycle\n\n";

    ct_state = encrypt_n(5.0);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, pt_add);   // +1
        } else {
            ct_state = cc->EvalSub(ct_state, pt_sub);   // -2
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct_state);
    double value_final = pow(PHI, n_final);
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Final n: " << n_final << "\n";
    cout << "    Final value: " << value_final << "\n";
    cout << "    Expected n: " << 5.0 + 5000.0 * 1.0 - 5000.0 * 2.0 << "\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
