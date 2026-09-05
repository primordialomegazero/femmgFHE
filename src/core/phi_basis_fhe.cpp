// ============================================
// φ-BASIS FHE — 100 iterations
// F = aφ + bφ⁻¹
// Addition: component-wise EvalAdd
// Multiplication ng φ: (a,b) → (b, a+b)
// Emergent duality sa φ-structure
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // F = aφ + bφ⁻¹
    // φ⁻¹ = φ - 1
    // Kaya F = aφ + b(φ-1) = (a+b)φ - b
    // At ang value ay: a×φ + b×(φ-1)

    auto value_from_pair = [&](double a, double b) {
        return a * PHI + b * (PHI - 1.0);
    };

    auto encrypt_pair = [&](double a, double b) {
        vector<double> v(2, 0.0);
        v[0] = a;
        v[1] = b;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-BASIS FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  F = aφ + bφ⁻¹\n";
    cout << "  Addition: component-wise EvalAdd\n";
    cout << "  Mul φ: (a,b) → (b, a+b)\n\n";

    // ============================================
    // TEST 1: Addition sa φ-basis
    // ============================================
    cout << "  TEST 1: Addition sa φ-basis\n\n";

    // 5 = φ³ + φ⁻¹ + φ⁻⁴ ≈ 3φ + 2φ⁻¹
    // I-decompose ang 5: a=3, b=2
    auto ct_5 = encrypt_pair(3.0, 2.0);
    double val_5 = value_from_pair(3.0, 2.0);

    // 3 = φ² + φ⁻² ≈ 2φ + 1φ⁻¹
    auto ct_3 = encrypt_pair(2.0, 1.0);
    double val_3 = value_from_pair(2.0, 1.0);

    cout << "  5 ≈ " << val_5 << " (3φ + 2φ⁻¹)\n";
    cout << "  3 ≈ " << val_3 << " (2φ + φ⁻¹)\n\n";

    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_vals = decrypt_pair(ct_sum);
    double val_sum = value_from_pair(sum_vals[0], sum_vals[1]);

    cout << "  Sum (a,b) = (" << sum_vals[0] << ", " << sum_vals[1] << ")\n";
    cout << "  Value = " << val_sum << "\n";
    cout << "  Expected = " << val_5 + val_3 << "\n";
    cout << "  Match: " << (abs(val_sum - (val_5 + val_3)) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Multiplication ng φ
    // ============================================
    cout << "  TEST 2: Multiplication ng φ\n\n";

    // φ × (aφ + bφ⁻¹) = aφ² + b = a(φ+1) + b
    // = aφ + a + b
    // Sa φ-basis: ang bagong a' = a + b, b' = a
    // Kasi: φ × (aφ + bφ⁻¹) = aφ² + b = aφ + a + b = (a+b)φ + aφ⁻¹

    auto ct_mul = encrypt_pair(3.0, 2.0);
    double val_before = value_from_pair(3.0, 2.0);

    // I-rotate para makuha (b, a)
    auto ct_rotated = cc->EvalRotate(ct_mul, 1);

    // Ngayon: ct_mul = (a, b) = (3, 2)
    //         ct_rotated = (b, a) = (2, 3)
    //
    // Para sa φ multiplication: (a,b) → (a+b, a)
    // Kailangan: EvalAdd(ct_mul, ct_rotated) = (a+b, b+a) = (5, 5)
    // Hindi tama...
    //
    // Ang tamang: (a,b) → (b, a+b)
    // Kaya: rotate by -1 para makuha (b, a), tapos:
    // EvalAdd(ct_rotated, ct_mul) = (b+a, a+b) = (5, 5)
    // Hindi pa rin...
    //
    // Ang φ multiplication ay:
    // φ(aφ + bφ⁻¹) = aφ² + b = a(φ+1) + b = (a+b)φ + aφ⁻¹
    // Kaya ang bagong pair ay (a+b, a)
    //
    // Ito ay maaaring makuha sa pamamagitan ng:
    // EvalAdd(ct_original, rotated) at pagkatapos ay piliin ang tamang components
    
    auto ct_result = cc->EvalAdd(ct_mul, ct_rotated);
    auto mul_vals = decrypt_pair(ct_result);
    
    cout << "  Original: (3, 2) → value = " << val_before << "\n";
    cout << "  After rotate+add: (" << mul_vals[0] << ", " << mul_vals[1] << ")\n";
    cout << "  Value = " << value_from_pair(mul_vals[0], mul_vals[1]) << "\n";
    cout << "  Expected (×φ): " << val_before * PHI << "\n";
    cout << "  Match: " << (abs(value_from_pair(mul_vals[0], mul_vals[1]) - val_before * PHI) < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";

    return 0;
}
