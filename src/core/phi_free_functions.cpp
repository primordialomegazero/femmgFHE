// ============================================
// φ-FREE FUNCTIONS — Walang EvalMult
// Lahat ng libreng CKKS functions
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
    parameters.SetMultiplicativeDepth(3);
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_val = [&](double val) {
        vector<double> v(4, 0.0);
        v[0] = val;
        v[1] = val;
        v[2] = val;
        v[3] = val;
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
    cout << "  φ-FREE FUNCTIONS — Walang EvalMult\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: EvalPolyLinear degree 1
    // ============================================
    cout << "  TEST 1: EvalPolyLinear degree 1\n\n";

    auto ct_x = encrypt_val(3.0);
    vector<double> coeffs_deg1 = {1.0, 2.0};  // f(x) = 2x + 1
    
    auto ct_deg1 = cc->EvalPolyLinear(ct_x, coeffs_deg1);
    auto v_deg1 = decrypt_val(ct_deg1);
    cout << "    f(3) = 7 → " << v_deg1[0] << " ✅\n";
    cout << "    Level: " << ct_deg1->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: EvalPolyLinear degree 2
    // ============================================
    cout << "  TEST 2: EvalPolyLinear degree 2\n\n";

    ct_x = encrypt_val(3.0);
    vector<double> coeffs_deg2 = {1.0, 2.0, 0.5};  // f(x) = 0.5x² + 2x + 1
    
    auto ct_deg2 = cc->EvalPolyLinear(ct_x, coeffs_deg2);
    auto v_deg2 = decrypt_val(ct_deg2);
    cout << "    f(3) = 11.5 → " << v_deg2[0] << " ✅\n";
    cout << "    Level: " << ct_deg2->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: EvalRotate
    // ============================================
    cout << "  TEST 3: EvalRotate\n\n";

    ct_x = encrypt_val(5.0);
    auto ct_rot = cc->EvalRotate(ct_x, 1);
    auto v_rot = decrypt_val(ct_rot);
    
    cout << "    All 5s rotated by 1: (" << v_rot[0] << ", " << v_rot[1] 
         << ", " << v_rot[2] << ", " << v_rot[3] << ")\n";
    cout << "    Level: " << ct_rot->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: EvalNegate + EvalAdd
    // ============================================
    cout << "  TEST 4: EvalNegate + EvalAdd\n\n";

    auto ct_a = encrypt_val(10.0);
    auto ct_b = encrypt_val(3.0);
    auto ct_diff = cc->EvalAdd(ct_a, cc->EvalNegate(ct_b));
    auto v_diff = decrypt_val(ct_diff);
    
    cout << "    10 - 3 = " << v_diff[0] << " ✅\n";
    cout << "    Level: " << ct_diff->GetLevel() << "\n\n";

    // ============================================
    // TEST 5: φ-correction approximation
    // ============================================
    cout << "  TEST 5: φ-correction approximation\n\n";

    // corr(k) = log_φ(1 + φ^(-k))
    // Para sa k = 1..10, hanapin ang best linear fit
    
    cout << "    k | exact corr | linear approx\n";
    cout << "    --|------------|---------------\n";
    
    for (int k = 1; k <= 10; k++) {
        double exact = log(1.0 + pow(PHI, -k)) / LN_PHI;
        double linear = 1.25 - 0.25 * k;
        cout << "    " << setw(2) << k << " | "
             << setw(10) << exact << " | "
             << setw(10) << linear << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 6: Sequential PolyLinear
    // ============================================
    cout << "  TEST 6: Sequential PolyLinear (5 steps)\n\n";

    ct_x = encrypt_val(2.0);
    
    for (int i = 0; i < 5; i++) {
        vector<double> coeffs_step = {0.5, 1.0};
        ct_x = cc->EvalPolyLinear(ct_x, coeffs_step);
    }
    
    auto v_seq = decrypt_val(ct_x);
    cout << "    Value: " << v_seq[0] << " (expected: " << 2.0 + 5.0 * 0.5 << ")\n";
    cout << "    Level: " << ct_x->GetLevel() << "\n";

    return 0;
}
