// ============================================
// φ-BASIS 3-SLOT — 100 iterations
// F = aφ + bφ⁻¹
// 3-slot: (a, b, 0)
// Mul φ: rotate+add → (a, a+b, b)
// Ang bagong pair ay (Slot 1, Slot 0)
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3});

    const double PHI = 1.6180339887498948482;

    auto value_from_pair = [&](double a, double b) {
        return a * PHI + b * (PHI - 1.0);
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
    cout << "  φ-BASIS 3-SLOT — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  F = aφ + bφ⁻¹\n";
    cout << "  State: (a, b, 0, 0)\n";
    cout << "  Mul φ: rotate+add → (a, a+b, b, 0)\n\n";

    // ============================================
    // TEST 1: Addition (component-wise)
    // ============================================
    cout << "  TEST 1: Addition\n\n";

    auto ct_5 = encrypt_triple(3.0, 2.0);
    auto ct_3 = encrypt_triple(2.0, 1.0);

    double val_5 = value_from_pair(3.0, 2.0);
    double val_3 = value_from_pair(2.0, 1.0);

    cout << "  5 ≈ " << val_5 << " (3φ + 2φ⁻¹)\n";
    cout << "  3 ≈ " << val_3 << " (2φ + φ⁻¹)\n";

    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_vals = decrypt_triple(ct_sum);

    cout << "  Sum slots: (" << sum_vals[0] << ", " << sum_vals[1] << ", " << sum_vals[2] << ")\n";
    cout << "  Value = " << value_from_pair(sum_vals[0], sum_vals[1]) << "\n";
    cout << "  Expected = " << val_5 + val_3 << "\n";
    cout << "  Match: " << (abs(value_from_pair(sum_vals[0], sum_vals[1]) - (val_5 + val_3)) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Multiplication ng φ
    // ============================================
    cout << "  TEST 2: Multiplication ng φ\n\n";

    auto ct_mul = encrypt_triple(3.0, 2.0);
    double val_before = value_from_pair(3.0, 2.0);

    cout << "  Original: (3, 2, 0) → value = " << val_before << "\n";

    // φ × (aφ + bφ⁻¹) = aφ² + b = a(φ+1) + b = (a+b)φ + aφ⁻¹
    // Kaya ang bagong pair ay (a+b, a)
    
    // I-rotate by 1: (0, 3, 2, 0) — ang slot 0 ay 0, slot 1 ay a, slot 2 ay b
    auto ct_rotated = cc->EvalRotate(ct_mul, 1);
    
    // EvalAdd: (3, 2, 0, 0) + (0, 3, 2, 0) = (3, 5, 2, 0)
    auto ct_result = cc->EvalAdd(ct_mul, ct_rotated);
    
    auto mul_vals = decrypt_triple(ct_result);
    
    cout << "  After rotate+add: (" << mul_vals[0] << ", " << mul_vals[1] << ", " << mul_vals[2] << ")\n";
    
    // Ang bagong pair ay (Slot 1, Slot 0) = (5, 3)
    double new_a = mul_vals[1];  // a+b = 5
    double new_b = mul_vals[0];  // a = 3
    
    double val_after = value_from_pair(new_a, new_b);
    double expected = val_before * PHI;
    
    cout << "  New pair: (" << new_a << ", " << new_b << ")\n";
    cout << "  Value = " << val_after << "\n";
    cout << "  Expected (×φ): " << expected << "\n";
    cout << "  Match: " << (abs(val_after - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: Sequence ng φ multiplications
    // ============================================
    cout << "  TEST 3: Sequence ng φ multiplications\n\n";

    auto ct_seq = encrypt_triple(1.0, 0.0);  // φ¹ = φ
    double val_seq = value_from_pair(1.0, 0.0);
    
    cout << "    Start: φ = " << val_seq << "\n";
    
    for (int i = 0; i < 5; i++) {
        auto rot = cc->EvalRotate(ct_seq, 1);
        ct_seq = cc->EvalAdd(ct_seq, rot);
        
        auto vals = decrypt_triple(ct_seq);
        double a = vals[1];  // a+b
        double b = vals[0];  // a
        
        val_seq = value_from_pair(a, b);
        cout << "    ×φ → " << val_seq << "\n";
    }
    
    cout << "\n  Level: " << ct_seq->GetLevel() << "\n";

    return 0;
}
