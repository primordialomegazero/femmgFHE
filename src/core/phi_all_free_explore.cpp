// ============================================
// φ-ALL FREE EXPLORE — Walang EvalMult
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
    cout << "  φ-ALL FREE EXPLORE\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: EvalPolyLinear na may degree 2
    // ============================================
    cout << "  TEST 1: EvalPolyLinear degree 2\n\n";

    auto ct_x = encrypt_val(3.0);
    vector<double> coeffs_deg2 = {1.0, 2.0, 0.5};  // f(x) = 0.5x² + 2x + 1
    
    try {
        auto ct_deg2 = cc->EvalPolyLinear(ct_x, coeffs_deg2);
        auto v_deg2 = decrypt_val(ct_deg2);
        cout << "    f(3) = 0.5(9) + 6 + 1 = " << v_deg2[0] << "\n";
        cout << "    Expected: 11.5\n";
        cout << "    Match: " << (abs(v_deg2[0] - 11.5) < 0.01 ? "✅" : "❌") << "\n";
        cout << "    Level: " << ct_deg2->GetLevel() << "\n\n";
    } catch (...) {
        cout << "    Failed\n\n";
    }

    // ============================================
    // TEST 2: EvalPolyLinear degree 3
    // ============================================
    cout << "  TEST 2: EvalPolyLinear degree 3\n\n";

    vector<double> coeffs_deg3 = {0.0, 0.0, 1.0, 0.0};  // f(x) = x²
    
    try {
        auto ct_deg3 = cc->EvalPolyLinear(ct_x, coeffs_deg3);
        auto v_deg3 = decrypt_val(ct_deg3);
        cout << "    f(3) = 9\n";
        cout << "    Match: " << (abs(v_deg3[0] - 9.0) < 0.01 ? "✅" : "❌") << "\n";
        cout << "    Level: " << ct_deg3->GetLevel() << "\n\n";
    } catch (...) {
        cout << "    Failed — kailangan ng higher degree\n\n";
    }

    // ============================================
    // TEST 3: EvalRotate para sa slot manipulation
    // ============================================
    cout << "  TEST 3: EvalRotate slot manipulation\n\n";

    ct_x = encrypt_val(5.0);
    
    auto ct_rot1 = cc->EvalRotate(ct_x, 1);
    auto ct_rot2 = cc->EvalRotate(ct_x, 2);
    
    auto v_rot1 = decrypt_val(ct_rot1);
    auto v_rot2 = decrypt_val(ct_rot2);
    
    cout << "    Original: (" << 5.0 << ", " << 5.0 << ", " << 5.0 << ", " << 5.0 << ")\n";
    cout << "    Rotate 1: (" << v_rot1[0] << ", " << v_rot1[1] << ", " << v_rot1[2] << ", " << v_rot1[3] << ")\n";
    cout << "    Rotate 2: (" << v_rot2[0] << ", " << v_rot2[1] << ", " << v_rot2[2] << ", " << v_rot2[3] << ")\n";
    cout << "    Level: " << ct_rot1->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: EvalAtIndex
    // ============================================
    cout << "  TEST 4: EvalAtIndex\n\n";

    ct_x = encrypt_val(7.0);
    
    try {
        auto ct_at = cc->EvalAtIndex(ct_x, 2);
        auto v_at = decrypt_val(ct_at);
        cout << "    At index 2: " << v_at[0] << "\n";
        cout << "    Level: " << ct_at->GetLevel() << "\n\n";
    } catch (...) {
        cout << "    Failed\n\n";
    }

    // ============================================
    // TEST 5: Multiple sequential PolyLinear
    // ============================================
    cout << "  TEST 5: Sequential PolyLinear\n\n";

    ct_x = encrypt_val(2.0);
    
    for (int i = 0; i < 5; i++) {
        vector<double> coeffs_step = {0.5, 1.0};  // f(x) = x + 0.5
        ct_x = cc->EvalPolyLinear(ct_x, coeffs_step);
    }
    
    auto v_seq = decrypt_val(ct_x);
    cout << "    After 5 iterations ng f(x) = x + 0.5:\n";
    cout << "    Value: " << v_seq[0] << "\n";
    cout << "    Expected: " << 2.0 + 5.0 * 0.5 << "\n";
    cout << "    Match: " << (abs(v_seq[0] - (2.0 + 5.0 * 0.5)) < 0.1 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct_x->GetLevel() << "\n\n";

    return 0;
}
