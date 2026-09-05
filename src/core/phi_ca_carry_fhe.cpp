// ============================================
// φ-CA CARRY FHE — Cellular Automaton Carry
// Rule: 11 → 00 + carry sa susunod
// Ang 2 sa sum ay natural na carry indicator
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
    cout << "  φ-CA CARRY FHE — Cellular Automaton\n";
    cout << "========================================\n\n";
    cout << "  Rule: 11 → 00 + carry sa susunod\n";
    cout << "  Ang 2 sa sum ay natural na carry indicator\n\n";

    // ============================================
    // TEST: 5 + 3 = 8
    // 5 = φ³ + φ⁻¹ + φ⁻⁴
    // 3 = φ² + φ⁻²
    // φ³ + φ² = φ⁴ (carry!)
    // ============================================
    cout << "  TEST: 5 + 3 = 8\n\n";

    // 5: φ³(pos 10) + φ⁻¹(pos 6) + φ⁻⁴(pos 3)
    vector<double> bits_5(16, 0.0);
    bits_5[10] = 1.0;  // φ³
    bits_5[6] = 1.0;   // φ⁻¹
    bits_5[3] = 1.0;   // φ⁻⁴

    // 3: φ²(pos 9) + φ⁻²(pos 5)
    vector<double> bits_3(16, 0.0);
    bits_3[9] = 1.0;   // φ²
    bits_3[5] = 1.0;   // φ⁻²

    auto ct_5 = encrypt_bits(bits_5);
    auto ct_3 = encrypt_bits(bits_3);

    // Step 1: Component-wise addition
    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_bits = decrypt_bits(ct_sum);
    
    cout << "  Step 1 — Addition:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << sum_bits[i];
    cout << "]\n";
    cout << "  Value: " << value_from_bits(sum_bits) << "\n\n";

    // Step 2: I-rotate at i-add para ma-detect ang 11 pattern
    auto ct_rot = cc->EvalRotate(ct_sum, 1);
    auto ct_detect = cc->EvalAdd(ct_sum, ct_rot);
    auto detect_bits = decrypt_bits(ct_detect);
    
    cout << "  Step 2 — Detection (rotate+add):\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << detect_bits[i];
    cout << "]\n";
    cout << "  Value: " << value_from_bits(detect_bits) << "\n\n";

    // Step 3: Ang 2 sa detection ay nangangahulugang carry
    // Ang carry: 2φ^i = φ^(i+1) + φ^(i-2)
    // Kaya kailangan nating i-resolve ang mga 2
    cout << "  Step 3 — Carry resolution:\n";
    cout << "  Ang 2φ^i ay kailangang i-split sa φ^(i+1) at φ^(i-2)\n\n";

    // Sa plaintext, ang 2 ay lumalabas sa specific positions
    // Sa FHE, kailangan nating i-split ang 2 gamit ang φ-identity
    
    // I-rotate para ilipat ang carry sa tamang positions
    auto ct_carry_fwd = cc->EvalRotate(ct_detect, 1);   // carry forward: φ^(i+1)
    auto ct_carry_bwd = cc->EvalRotate(ct_detect, -2);  // carry backward: φ^(i-2)
    
    // I-combine
    auto ct_result = cc->EvalAdd(ct_sum, ct_carry_fwd);
    ct_result = cc->EvalAdd(ct_result, ct_carry_bwd);
    
    auto result_bits = decrypt_bits(ct_result);
    
    cout << "  Step 4 — Result:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << result_bits[i];
    cout << "]\n";
    cout << "  Value: " << value_from_bits(result_bits) << "\n";
    cout << "  Expected: 8\n\n";
    
    cout << "  Level: " << ct_result->GetLevel() << "\n";

    return 0;
}
