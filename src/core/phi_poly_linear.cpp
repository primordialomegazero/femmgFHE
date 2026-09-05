// ============================================
// φ-POLY LINEAR — Exploration
// EvalPolyLinear para sa arbitrary computation
// Walang depth reduction
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
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_val = [&](double val) {
        vector<double> v(4, 0.0);
        v[0] = val;
        v[1] = val * PHI;
        v[2] = val / PHI;
        v[3] = val * val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-POLY LINEAR — Exploration\n";
    cout << "========================================\n\n";
    cout << "  EvalPolyLinear para sa arbitrary ops\n";
    cout << "  Walang depth reduction\n\n";

    // ============================================
    // TEST 1: EvalPolyLinear — f(x) = 2x + 1
    // ============================================
    cout << "  TEST 1: EvalPolyLinear f(x) = 2x + 1\n\n";

    auto ct_x = encrypt_val(3.0);
    vector<double> coeffs = {1.0, 2.0};

    auto ct_poly = cc->EvalPolyLinear(ct_x, coeffs);
    auto v_poly = decrypt_val(ct_poly);

    cout << "  f(3) = 2(3) + 1 = " << v_poly[0] << "\n";
    cout << "  Expected: 7\n";
    cout << "  Match: " << (abs(v_poly[0] - 7.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: EvalNegate
    // ============================================
    cout << "  TEST 2: EvalNegate\n\n";

    ct_x = encrypt_val(5.0);
    auto ct_neg = cc->EvalNegate(ct_x);
    auto v_neg = decrypt_val(ct_neg);

    cout << "  -5 = " << v_neg[0] << "\n";
    cout << "  Expected: -5\n";
    cout << "  Match: " << (abs(v_neg[0] + 5.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: Subtraction via EvalNegate
    // ============================================
    cout << "  TEST 3: Subtraction via EvalNegate\n\n";

    auto ct_a = encrypt_val(10.0);
    auto ct_b = encrypt_val(3.0);

    auto ct_neg_b = cc->EvalNegate(ct_b);
    auto ct_diff = cc->EvalAdd(ct_a, ct_neg_b);

    auto v_diff = decrypt_val(ct_diff);
    cout << "  10 - 3 = " << v_diff[0] << "\n";
    cout << "  Expected: 7\n";
    cout << "  Match: " << (abs(v_diff[0] - 7.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: φ-correction via PolyLinear
    // ============================================
    cout << "  TEST 4: φ-correction via PolyLinear\n\n";
    cout << "  corr(k) ≈ a × k + b\n\n";

    // Ang correction para sa φ-power addition
    // corr(k) = log_φ(1 + φ^(-k))
    // Para sa k = 1..5, approximate bilang linear function
    
    ct_x = encrypt_val(3.0);
    
    // corr(k) ≈ -0.25 × k + 1.25 (approximation)
    vector<double> corr_coeffs = {1.25, -0.25};
    
    auto ct_corr = cc->EvalPolyLinear(ct_x, corr_coeffs);
    auto v_corr = decrypt_val(ct_corr);
    
    cout << "  corr(3) ≈ " << v_corr[0] << "\n";
    cout << "  Exact corr(3) = " << log(1.0 + pow(PHI, -3)) / LN_PHI << "\n";
    cout << "  Match: " << (abs(v_corr[0] - log(1.0 + pow(PHI, -3)) / LN_PHI) < 0.1 ? "✅" : "❌") << "\n\n";
    
    cout << "  Level: " << ct_corr->GetLevel() << "\n";

    return 0;
}
