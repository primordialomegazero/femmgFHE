// ============================================
// φ-TAYLOR CORRECTION FHE — 50 iterations
// Pre-computed corrections mula sa φ-series
// Arbitrary addition na walang decrypt
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
    const double C = 1.0 / LN_PHI;

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

    // Taylor series correction para sa corr(k)
    auto taylor_corr = [&](int k, int terms = 5) {
        double sum = 0;
        for (int j = 1; j <= terms; j++) {
            double sign = (j % 2 == 1) ? 1.0 : -1.0;
            sum += sign * C / j * pow(PHI, -j * k);
        }
        return sum;
    };

    cout << "========================================\n";
    cout << "  φ-TAYLOR CORRECTION FHE — 50 iterations\n";
    cout << "========================================\n\n";
    cout << "  Pre-computed corrections mula sa φ-series\n";
    cout << "  Arbitrary addition na walang decrypt\n\n";

    // ============================================
    // TEST: φ⁴ + 5 gamit ang Taylor corrections
    // ============================================
    cout << "  TEST: φ⁴ + 5 = 11.854\n\n";

    double n_start = 4.0;
    auto ct_state = encrypt_n(n_start);
    double current_value = pow(PHI, n_start);

    // Ang +5 ay: 5 = φ³ + φ⁻¹ + φ⁻⁴
    // Sa φ⁴ space, ang ratios ay:
    // φ³/φ⁴ = φ⁻¹ → k=1
    // φ⁻¹/φ⁴ = φ⁻⁵ → k=5
    // φ⁻⁴/φ⁴ = φ⁻⁸ → k=8
    
    double corr1 = taylor_corr(1);
    double corr2 = taylor_corr(5);
    double corr3 = taylor_corr(8);
    
    cout << "  Corrections: corr(1)=" << corr1 
         << ", corr(5)=" << corr2 
         << ", corr(8)=" << corr3 << "\n\n";
    
    // I-apply ang corrections
    vector<double> delta_v(1, 0.0);
    Plaintext pt_delta;
    
    delta_v[0] = corr1;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    delta_v[0] = corr2;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    delta_v[0] = corr3;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n_after = decrypt_n(ct_state);
    double value_after = pow(PHI, n_after);
    double expected = current_value + 5.0;
    
    cout << "  Result: " << value_after << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(value_after - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: Sequence ng additions
    // ============================================
    cout << "  TEST: Sequence ng additions\n";
    cout << "  φ⁴ + 5 - 2 + 3 = 12.854\n\n";

    ct_state = encrypt_n(n_start);
    current_value = pow(PHI, n_start);
    
    // +5
    delta_v[0] = corr1;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    delta_v[0] = corr2;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    delta_v[0] = corr3;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n1 = decrypt_n(ct_state);
    double v1 = pow(PHI, n1);
    cout << "  +5 → " << v1 << "\n";
    
    // -2: 2 = φ² + φ⁻²
    // Sa current value (v1), ang ratios:
    // φ²/v1 = φ^(2-4) = φ⁻² → k=2
    // φ⁻²/v1 = φ^(-2-4) = φ⁻⁶ → k=6
    double corr_sub1 = -taylor_corr(2);
    double corr_sub2 = -taylor_corr(6);
    
    delta_v[0] = corr_sub1;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    delta_v[0] = corr_sub2;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n2 = decrypt_n(ct_state);
    double v2 = pow(PHI, n2);
    cout << "  -2 → " << v2 << "\n";
    
    // +3: 3 = φ² + φ⁻²
    // Sa current value (v2), ang ratios ay papalapit sa k=2 at k=6
    delta_v[0] = taylor_corr(2);
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    delta_v[0] = taylor_corr(6);
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n3 = decrypt_n(ct_state);
    double v3 = pow(PHI, n3);
    cout << "  +3 → " << v3 << "\n";
    
    cout << "  Expected final: " << current_value + 5 - 2 + 3 << "\n";
    cout << "  Match: " << (abs(v3 - (current_value + 5 - 2 + 3)) < 0.5 ? "✅" : "❌") << "\n\n";
    
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
