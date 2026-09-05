// ============================================
// φ-MATRIX TRANSFORM — Elegant Carry
// φ × (a,b) = (a+b, a)
// 3-slot: (a, b, 0) → rotate+add → (a+b, b, a)
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
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;

    auto value_from_pair = [&](double a, double b) {
        return a + b * PHI;
    };

    auto encrypt_triple = [&](double a, double b) {
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = 0.0;
        v[3] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_triple = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-MATRIX TRANSFORM — Elegant Carry\n";
    cout << "========================================\n\n";
    cout << "  φ × (a,b) = (a+b, a)\n";
    cout << "  3-slot: (a, b, 0) → rotate+add\n\n";

    // ============================================
    // TEST 1: φ¹ = (0, 1)
    // ============================================
    cout << "  TEST 1: φ¹ = (0, 1)\n\n";

    auto ct = encrypt_triple(0.0, 1.0);
    double val = value_from_pair(0.0, 1.0);
    
    cout << "    Start: (" << 0.0 << ", " << 1.0 << ") = " << val << "\n";

    // φ × (0,1) = (1, 0) — dapat maging φ²
    auto ct_rot = cc->EvalRotate(ct, 1);
    auto ct_result = cc->EvalAdd(ct, ct_rot);
    
    auto v = decrypt_triple(ct_result);
    cout << "    After rotate+add: (" << v[0] << ", " << v[1] << ", " << v[2] << ")\n";
    
    // Ang bagong pair ay (v[1], v[0]) = (1, 0)
    double new_a = v[1];
    double new_b = v[0];
    cout << "    New pair: (" << new_a << ", " << new_b << ") = " << value_from_pair(new_a, new_b) << "\n";
    cout << "    Expected φ²: " << pow(PHI, 2) << "\n";
    cout << "    Match: " << (abs(value_from_pair(new_a, new_b) - pow(PHI, 2)) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Sequence ng φ multiplications
    // ============================================
    cout << "  TEST 2: Sequence ng φ multiplications\n\n";

    ct = encrypt_triple(0.0, 1.0);
    cout << "    Start: φ¹ = " << value_from_pair(0.0, 1.0) << "\n";

    for (int i = 0; i < 5; i++) {
        ct_rot = cc->EvalRotate(ct, 1);
        ct = cc->EvalAdd(ct, ct_rot);
        
        auto vv = decrypt_triple(ct);
        double a = vv[1];
        double b = vv[0];
        double val_now = value_from_pair(a, b);
        
        cout << "    ×φ → (" << a << ", " << b << ") = " << val_now << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 3: Ang carry bilang φ-multiplication
    // ============================================
    cout << "  TEST 3: Ang carry bilang φ-multiplication\n\n";
    cout << "  φ³ + φ² = φ⁴\n";
    cout << "  (1,2) + (1,1) = (2,3) → φ⁴\n\n";

    // φ³ = (1, 2)
    auto ct_phi3 = encrypt_triple(1.0, 2.0);
    // φ² = (1, 1)
    auto ct_phi2 = encrypt_triple(1.0, 1.0);

    auto ct_carry = cc->EvalAdd(ct_phi3, ct_phi2);
    auto v_carry = decrypt_triple(ct_carry);

    cout << "    After addition: (" << v_carry[0] << ", " << v_carry[1] << ")\n";
    cout << "    Value: " << value_from_pair(v_carry[0], v_carry[1]) << "\n";
    cout << "    Expected φ⁴: " << pow(PHI, 4) << "\n";
    cout << "    Match: " << (abs(value_from_pair(v_carry[0], v_carry[1]) - pow(PHI, 4)) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_carry->GetLevel() << "\n";

    return 0;
}
