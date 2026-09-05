// ============================================
// φ-LINEAR TRANSFORM — EvalLinearTransform
// Ang matrix [[1,1],[1,0]] bilang linear transform
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
    cc->Enable(FHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;

    auto value_from_pair = [&](double a, double b) {
        return a + b * PHI;
    };

    auto encrypt_pair = [&](double a, double b) {
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = 0.0;
        v[3] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-LINEAR TRANSFORM\n";
    cout << "========================================\n\n";
    cout << "  Matrix [[1,1],[1,0]] bilang linear transform\n";
    cout << "  φ × (a,b) = (a+b, a)\n\n";

    // ============================================
    // TEST 1: EvalSum
    // ============================================
    cout << "  TEST 1: EvalSum\n\n";

    auto ct = encrypt_pair(1.0, 2.0);
    auto v_before = decrypt_pair(ct);
    cout << "    Before: (" << v_before[0] << ", " << v_before[1] << ")\n";

    try {
        auto ct_sum = cc->EvalSum(ct, 4);
        auto v_sum = decrypt_pair(ct_sum);
        cout << "    After EvalSum: (" << v_sum[0] << ", " << v_sum[1] << ", " << v_sum[2] << ", " << v_sum[3] << ")\n";
        cout << "    Level: " << ct_sum->GetLevel() << "\n\n";
    } catch (const exception& e) {
        cout << "    EvalSum failed: " << e.what() << "\n\n";
    }

    // ============================================
    // TEST 2: EvalInnerProduct
    // ============================================
    cout << "  TEST 2: EvalInnerProduct\n\n";

    try {
        auto ct_a = encrypt_pair(1.0, 2.0);
        auto ct_b = encrypt_pair(1.0, 1.0);
        auto ct_inner = cc->EvalInnerProduct(ct_a, ct_b, 4);
        auto v_inner = decrypt_pair(ct_inner);
        cout << "    Inner product: " << v_inner[0] << "\n";
        cout << "    Expected: 1×1 + 2×1 = 3\n";
        cout << "    Level: " << ct_inner->GetLevel() << "\n\n";
    } catch (const exception& e) {
        cout << "    EvalInnerProduct failed: " << e.what() << "\n\n";
    }

    // ============================================
    // TEST 3: Manual matrix transform gamit rotate
    // ============================================
    cout << "  TEST 3: Manual matrix transform\n\n";
    cout << "  (a,b) → (a+b, a)\n";
    cout << "  Sa 2-slot na may rotate:\n\n";

    ct = encrypt_pair(1.0, 2.0);
    cout << "    Start: (1, 2) = " << value_from_pair(1, 2) << "\n";

    // I-rotate by 1: (b, a)
    auto ct_rot = cc->EvalRotate(ct, 1);
    auto v_rot = decrypt_pair(ct_rot);
    cout << "    Rotate: (" << v_rot[0] << ", " << v_rot[1] << ")\n";

    // I-add: (a+b, b+a) = (3, 3)
    auto ct_add = cc->EvalAdd(ct, ct_rot);
    auto v_add = decrypt_pair(ct_add);
    cout << "    Add: (" << v_add[0] << ", " << v_add[1] << ")\n";

    // Ang problema: pareho silang 3
    // Kailangan: (3, 1) — Slot 0 = a+b, Slot 1 = a
    
    // Subukan: gamitin ang rotate by 2
    auto ct_rot2 = cc->EvalRotate(ct, 2);
    auto v_rot2 = decrypt_pair(ct_rot2);
    cout << "    Rotate 2: (" << v_rot2[0] << ", " << v_rot2[1] << ")\n";

    cout << "\n  Level: " << ct_add->GetLevel() << "\n";

    return 0;
}
