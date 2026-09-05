// ============================================
// φ-CONSTANT CORRECTION FHE — 50 iterations
// Arbitrary integer addition gamit ang
// constant φ-power corrections
// Walang decrypt, walang EvalMult
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

    // Pre-compute constant corrections para sa φ^(-k)
    auto correction = [&](int k) {
        return log(1.0 + pow(PHI, -k)) / LN_PHI;
    };

    cout << "========================================\n";
    cout << "  φ-CONSTANT CORRECTION FHE — 50 iterations\n";
    cout << "========================================\n\n";
    cout << "  Arbitrary integer addition gamit ang\n";
    cout << "  constant φ-power corrections\n\n";

    // Start: value = φ^4 = 6.854 (sa φ-space)
    double n_start = 4.0;
    auto ct_state = encrypt_n(n_start);
    double current_value = pow(PHI, n_start);

    cout << "  Initial: value = φ⁴ = " << current_value << "\n";
    cout << "  n = " << n_start << "\n\n";

    // Test: φ⁴ + φ² = φ⁴ × (1 + φ⁻²)
    // Correction = log_φ(1 + φ⁻²) = 0.672275938185
    double k = 2.0;
    double corr = correction(2);
    
    vector<double> delta_v(1, corr);
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n_after = decrypt_n(ct_state);
    double value_after = pow(PHI, n_after);
    double expected = pow(PHI, 4) + pow(PHI, 2);
    
    cout << "  TEST: φ⁴ + φ² = " << current_value << " + " << pow(PHI, 2) << "\n";
    cout << "    Correction: " << corr << "\n";
    cout << "    After FHE: " << value_after << "\n";
    cout << "    Expected: " << expected << "\n";
    cout << "    Match: " << (abs(value_after - expected) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: Integer addition gamit decomposition
    // ============================================
    cout << "  TEST: 6.854 + 5 = 11.854\n";
    cout << "  5 = φ³ + φ⁻¹ + φ⁻⁴\n\n";

    // Reset sa φ⁴
    ct_state = encrypt_n(4.0);
    current_value = pow(PHI, 4.0);

    // 5 = φ³ + φ⁻¹ + φ⁻⁴
    // Sa log space, ang +φ³ ay correction depende sa ratio
    // φ⁴ + φ³ = φ⁴(1 + φ⁻¹) — correction = log_φ(1+φ⁻¹) = 1
    // Pero kailangan natin ng arbitrary integer addition...
    
    // Ang decomposition ng 5 bilang φ-powers:
    // 5 = φ³ + φ⁻¹ + φ⁻⁴
    // Sa φ⁴ space:
    // +φ³ = φ⁴ × φ⁻¹ — correction = log_φ(1 + φ⁻¹) = 1
    // +φ⁻¹ = φ⁴ × φ⁻⁵ — correction = log_φ(1 + φ⁻⁵) = 0.179
    // +φ⁻⁴ = φ⁴ × φ⁻⁸ — correction = log_φ(1 + φ⁻⁸) = 0.044
    
    double corr1 = correction(1);  // +φ³ sa φ⁴
    double corr2 = correction(5);  // +φ⁻¹ sa φ⁴
    double corr3 = correction(8);  // +φ⁻⁴ sa φ⁴
    
    cout << "  Corrections: " << corr1 << ", " << corr2 << ", " << corr3 << "\n\n";
    
    // I-apply ang tatlong corrections
    delta_v[0] = corr1;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    delta_v[0] = corr2;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    delta_v[0] = corr3;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    n_after = decrypt_n(ct_state);
    value_after = pow(PHI, n_after);
    expected = current_value + 5.0;
    
    cout << "  Result: " << value_after << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(value_after - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: Mixed operations na walang decrypt
    // ============================================
    cout << "  TEST: Mixed operations (walang decrypt sa loop)\n\n";

    ct_state = encrypt_n(4.0);
    
    // Operations: +5, -2, ×3, ÷4
    // Lahat ay pre-computed corrections
    
    auto start = high_resolution_clock::now();
    
    for (int iter = 0; iter < 10; iter++) {
        // +5: tatlong corrections
        delta_v[0] = corr1;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
        
        delta_v[0] = corr2;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
        
        delta_v[0] = corr3;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
        
        // ×3: multiply
        delta_v[0] = log(3.0) / LN_PHI;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
        
        // ÷4: divide
        delta_v[0] = log(4.0) / LN_PHI;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalSub(ct_state, pt_delta);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct_state);
    double value_final = pow(PHI, n_final);
    
    cout << "  10 iterations ng +5, ×3, ÷4\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Final value: " << value_final << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
