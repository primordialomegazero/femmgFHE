// ============================================
// φ-LINEAR TRANSFORM MATRIX
// [[0,1],[1,1]] × (a,b) = (b, a+b)
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

    auto encrypt_quad = [&](double a, double b) {
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = 0.0;
        v[3] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_quad = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-LINEAR TRANSFORM MATRIX\n";
    cout << "========================================\n\n";
    cout << "  [[0,1],[1,1]] × (a,b) = (b, a+b)\n\n";

    // ============================================
    // TEST 1: Manual linear transform gamit rotate
    // ============================================
    cout << "  TEST 1: Manual approach\n\n";

    auto ct = encrypt_quad(1.0, 2.0);
    cout << "    Start: (1, 2) = φ³ = " << value_from_pair(1, 2) << "\n";

    // Ang kailangan: (2, 3) = (b, a+b)
    // Slot 0 = 2 (mula sa lumang Slot 1)
    // Slot 1 = 3 (mula sa lumang Slot 0 + 1)

    // I-rotate by 1: (2, 0, 0, 1) — ang lumang b ay nasa Slot 0
    auto ct_rot = cc->EvalRotate(ct, 1);
    auto v_rot = decrypt_quad(ct_rot);
    cout << "    Rotate(1): (" << v_rot[0] << ", " << v_rot[1] << ")\n";

    // Ang lumang a ay nasa Slot 1 ng rotated
    // Kaya ang a+b ay dapat mula sa Slot 0 + Slot 1 ng original
    
    // I-rotate by -1: (0, 1, 2, 0) — ang lumang a ay nasa Slot 1
    auto ct_rot_back = cc->EvalRotate(ct, -1);
    auto v_rot_back = decrypt_quad(ct_rot_back);
    cout << "    Rotate(-1): (" << v_rot_back[0] << ", " << v_rot_back[1] << ")\n";

    // I-add ang rotate(1) at rotate(-1):
    // rotate(1) = (b, 0, 0, a)
    // rotate(-1) = (0, a, b, 0)
    // add = (b, a, b, a)
    auto ct_combined = cc->EvalAdd(ct_rot, ct_rot_back);
    auto v_combined = decrypt_quad(ct_combined);
    cout << "    Rotate(1)+Rotate(-1): (" << v_combined[0] << ", " << v_combined[1] << ")\n";

    // Ang Slot 0 ay b = 2 ✅
    // Ang Slot 1 ay a = 1 — kailangan a+b = 3
    
    // I-add ang original sa combined:
    // original = (a, b, 0, 0) = (1, 2, 0, 0)
    // combined = (b, a, b, a) = (2, 1, 2, 1)
    // add = (a+b, a+b, b, a) = (3, 3, 2, 1)
    auto ct_final = cc->EvalAdd(ct, ct_combined);
    auto v_final = decrypt_quad(ct_final);
    cout << "    Final: (" << v_final[0] << ", " << v_final[1] << ") = "
         << value_from_pair(v_final[0], v_final[1]) << "\n";
    cout << "    Expected φ⁴: " << pow(PHI, 4) << "\n";
    cout << "    Match: " << (abs(value_from_pair(v_final[0], v_final[1]) - pow(PHI, 4)) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Sequence ng φ-powers
    // ============================================
    cout << "  TEST 2: Sequence ng φ-powers\n\n";

    ct = encrypt_quad(0.0, 1.0);
    cout << "    φ¹ = " << value_from_pair(0, 1) << "\n";

    for (int i = 0; i < 6; i++) {
        auto v_before = decrypt_quad(ct);
        double a = v_before[0];
        double b = v_before[1];
        
        // Transition: (a, b) → (b, a+b)
        double new_a = b;
        double new_b = a + b;
        
        ct = encrypt_quad(new_a, new_b);
        cout << "    ×φ → (" << new_a << ", " << new_b << ") = "
             << value_from_pair(new_a, new_b) << "\n";
    }
    cout << "\n";

    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
