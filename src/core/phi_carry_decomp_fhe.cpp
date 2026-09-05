// ============================================
// φ-CARRY DECOMP FHE — Pure FHE carry
// 2φ^i = φ^(i+1) + φ^(i-2)
// Carry decomposition na walang comparison
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
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;
    const int MAX_POWER = 7;
    const int VEC_SIZE = 15;

    auto encrypt_bits = [&](const vector<double>& bits) {
        vector<double> v(16, 0.0);
        for (size_t i = 0; i < bits.size() && i < 16; i++) {
            v[i] = bits[i];
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(VEC_SIZE);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < VEC_SIZE; i++) {
            out.push_back(res[i].real());
        }
        return out;
    };

    auto value_from_bits = [&](const vector<double>& bits) {
        double val = 0;
        for (size_t i = 0; i < bits.size(); i++) {
            if (abs(bits[i]) > 0.5) {
                val += bits[i] * pow(PHI, (int)i - MAX_POWER);
            }
        }
        return val;
    };

    cout << "========================================\n";
    cout << "  φ-CARRY DECOMP FHE — Pure FHE carry\n";
    cout << "========================================\n\n";
    cout << "  2φ^i = φ^(i+1) + φ^(i-2)\n";
    cout << "  Carry decomposition na walang comparison\n\n";

    // ============================================
    // TEST 1: 5 + 3 = 8
    // ============================================
    cout << "  TEST 1: 5 + 3 = 8\n\n";

    // 5 = φ³ + φ⁻¹ + φ⁻⁴
    // 3 = φ² + φ⁻²
    // Sa vector (MAX_POWER=7):
    // φ³ → position 10, φ⁻¹ → position 6, φ⁻⁴ → position 3
    // φ² → position 9, φ⁻² → position 5

    vector<double> bits_5(16, 0.0);
    bits_5[10] = 1.0;  // φ³
    bits_5[6] = 1.0;   // φ⁻¹
    bits_5[3] = 1.0;   // φ⁻⁴

    vector<double> bits_3(16, 0.0);
    bits_3[9] = 1.0;   // φ²
    bits_3[5] = 1.0;   // φ⁻²

    auto ct_5 = encrypt_bits(bits_5);
    auto ct_3 = encrypt_bits(bits_3);

    // Component-wise addition
    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_bits = decrypt_bits(ct_sum);

    cout << "  After addition:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) {
        cout << setw(3) << sum_bits[i];
    }
    cout << "]\n";
    cout << "  Value: " << value_from_bits(sum_bits) << " (expected: 8)\n\n";

    // ============================================
    // TEST 2: Carry decomposition
    // ============================================
    cout << "  TEST 2: Carry decomposition\n\n";
    cout << "  φ³ + φ² = φ⁴ (carry forward)\n";
    cout << "  Ito ay emergent sa φ-structure\n\n";

    // Ang φ³ + φ² = φ⁴ — kaya ang position 10 at 9
    // ay dapat mag-carry papunta sa position 11
    
    // Carry rule: kung ang position i at i-1 ay parehong 1,
    // ang carry ay papunta sa position i+1
    //
    // Sa FHE:
    // 1. I-align ang position i sa i-1 (rotate by 1)
    // 2. I-add — kung pareho silang 1, ang sum ay 2
    // 3. Ang 2 ay nangangahulugang carry
    
    auto ct_rot = cc->EvalRotate(ct_sum, 1);
    auto ct_aligned = cc->EvalAdd(ct_sum, ct_rot);
    auto aligned_bits = decrypt_bits(ct_aligned);
    
    cout << "  After rotate+add:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) {
        cout << setw(3) << aligned_bits[i];
    }
    cout << "]\n";
    cout << "  Value: " << value_from_bits(aligned_bits) << "\n\n";

    // ============================================
    // TEST 3: Iterative carry
    // ============================================
    cout << "  TEST 3: Iterative carry\n\n";
    cout << "  Paulit-ulit na rotate+add para sa\n";
    cout << "  kumpletong carry propagation\n\n";

    auto ct_carry = ct_sum;
    
    for (int iter = 0; iter < 5; iter++) {
        // I-rotate by +1 para i-align ang φ^i sa φ^(i-1)
        auto ct_rot_fwd = cc->EvalRotate(ct_carry, 1);
        
        // I-add — ang sum ng φ^i at φ^(i-1) ay φ^(i+1)
        ct_carry = cc->EvalAdd(ct_carry, ct_rot_fwd);
        
        auto v = decrypt_bits(ct_carry);
        cout << "    Iter " << iter << ": value = " << value_from_bits(v) << "\n";
    }
    
    cout << "\n  Level: " << ct_carry->GetLevel() << "\n";

    return 0;
}
