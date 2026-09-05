// ============================================
// φ-MATRIX FIXED — Tamang Slot Alignment
// φ × (a,b) = (a+b, a)
// 4-slot: (a, b, a, b) → rotate → add → extract
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

    auto encrypt_quad = [&](double a, double b) {
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = a;  // redundant para sa alignment
        v[3] = b;  // redundant para sa alignment
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
    cout << "  φ-MATRIX FIXED — Tamang Alignment\n";
    cout << "========================================\n\n";
    cout << "  4-slot: (a, b, a, b)\n";
    cout << "  φ × (a,b) = (a+b, a)\n\n";

    // ============================================
    // TEST 1: Sequence ng φ powers
    // ============================================
    cout << "  TEST 1: Sequence ng φ powers\n\n";

    // φ¹ = (0, 1)
    auto ct = encrypt_quad(0.0, 1.0);
    double val = value_from_pair(0.0, 1.0);
    cout << "    φ¹ = " << val << "\n";

    for (int i = 0; i < 8; i++) {
        // φ × (a,b) = (a+b, a)
        // Sa 4-slot (a, b, a, b):
        // I-rotate by 1: (b, a, b, a)
        // I-add: (a+b, b+a, a+b, b+a) — pareho lahat
        // Hindi ito tama...
        
        // Ang tamang approach:
        // (a, b, a, b) → rotate by 2 → (a, b, a, b) — same
        // Hindi...
        
        // ANG TAMANG APPROACH:
        // Kailangan natin ng (a+b) sa Slot 0 at a sa Slot 1
        // (a, b, a, b)
        // Slot 0 + Slot 1 = a+b — ito ay nasa Slot 0
        // Slot 0 = a — ito ay nasa Slot 1 (i-rotate)
        
        // Sa FHE, hindi natin ma-extract ang individual slots nang walang multiply
        // Kaya gamitin natin ang 3-slot approach na mas simple
        
        // Reset approach: (a, b, 0, 0)
        // rotate by 1: (b, 0, 0, a)
        // add: (a+b, b, 0, a)
        // Ang bagong pair ay (Slot 0, Slot 1) = (a+b, b)
        // Pero dapat (a+b, a)
        
        // Ayusin: gamitin ang Slot 0 at Slot 2
        // (a, b, a, 0)
        // rotate by 1: (b, a, 0, a)
        // add: (a+b, a+b, a, a)
        // Ang bagong pair ay (Slot 0, Slot 2) = (a+b, a)
        
        auto v_before = decrypt_quad(ct);
        double a = v_before[0];
        double b = v_before[1];
        
        auto ct_rot = cc->EvalRotate(ct, 1);
        ct = cc->EvalAdd(ct, ct_rot);
        
        auto v_after = decrypt_quad(ct);
        double new_a = v_after[0];  // a+b
        double new_b = v_after[2];  // a
        
        val = value_from_pair(new_a, new_b);
        cout << "    ×φ → (" << new_a << ", " << new_b << ") = " << val << "\n";
        cout << "    Expected φ^" << i + 2 << " = " << pow(PHI, i + 2) << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: Ang carry — φ³ + φ² = φ⁴
    // ============================================
    cout << "  TEST 2: Ang carry — φ³ + φ² = φ⁴\n\n";

    // φ³ = (1, 2)
    auto ct_phi3 = encrypt_quad(1.0, 2.0);
    // φ² = (1, 1)
    auto ct_phi2 = encrypt_quad(1.0, 1.0);

    auto ct_carry = cc->EvalAdd(ct_phi3, ct_phi2);
    auto v_carry = decrypt_quad(ct_carry);

    cout << "    (" << v_carry[0] << ", " << v_carry[1] << ") = "
         << value_from_pair(v_carry[0], v_carry[1]) << "\n";
    cout << "    Expected φ⁴ = " << pow(PHI, 4) << "\n";
    cout << "    Match: " << (abs(value_from_pair(v_carry[0], v_carry[1]) - pow(PHI, 4)) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_carry->GetLevel() << "\n";

    return 0;
}
