// ============================================
// φ-CARRY FHE — Pure FHE carry propagation
// 2φ^i = φ^(i+1) + φ^(i-2)
// Walang decrypt, walang EvalMult
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, -1, -2});

    const double PHI = 1.6180339887498948482;
    const int MAX_POWER = 7;
    const int VEC_SIZE = 15;

    auto encrypt_bits = [&](const vector<double>& bits) {
        vector<double> v(16, 0.0);
        for (size_t i = 0; i < bits.size(); i++) v[i] = bits[i];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(VEC_SIZE);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < VEC_SIZE; i++) out.push_back(res[i].real());
        return out;
    };

    auto value_from_bits = [&](const vector<double>& bits) {
        double val = 0;
        for (size_t i = 0; i < bits.size(); i++) {
            if (bits[i] > 0.5) {
                val += pow(PHI, (int)i - MAX_POWER);
            }
        }
        return val;
    };

    cout << "========================================\n";
    cout << "  φ-CARRY FHE — Pure FHE carry\n";
    cout << "========================================\n\n";
    cout << "  Carry: 2φ^i = φ^(i+1) + φ^(i-2)\n\n";

    // State na may 2φ^i — kailangan ng carry
    // φ³ + φ² = φ⁴ (ito ang carry)
    // Sa vector: position ng φ³ ay 10, φ² ay 9
    vector<double> state(16, 0.0);
    state[10] = 1.0;  // φ³
    state[9] = 1.0;   // φ²
    // Ang φ³ + φ² = φ⁴ — dapat mag-carry sa φ⁴ (position 11)

    cout << "  Initial state:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << state[i];
    cout << "]\n";
    cout << "  Value: " << value_from_bits(vector<double>(state.begin(), state.begin()+VEC_SIZE)) << "\n\n";

    auto ct = encrypt_bits(state);

    // Carry propagation: 2φ^i = φ^(i+1) + φ^(i-2)
    // Ang φ³ + φ² ay hindi 2φ^i — magkaibang powers!
    // Kaya walang carry...
    //
    // Pero φ³ + φ² = φ⁴ ay emergent property
    // Ito ay maaaring i-apply bilang:
    // φ^i + φ^(i-1) = φ^(i+1)
    
    cout << "  Carry rule: φ^i + φ^(i-1) = φ^(i+1)\n\n";

    // Sa FHE:
    // 1. I-rotate para i-align ang φ^i sa φ^(i-1)
    // 2. EvalAdd para makuha ang sum
    // 3. Ang sum ay φ^(i+1) — i-rotate papunta sa tamang position
    
    auto ct_rot = cc->EvalRotate(ct, 1);  // shift pababa
    auto ct_carry = cc->EvalAdd(ct, ct_rot);
    
    auto v_carry = decrypt_bits(ct_carry);
    
    cout << "  After rotate+add:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << v_carry[i];
    cout << "]\n";
    cout << "  Value: " << value_from_bits(v_carry) << "\n";
    cout << "  Level: " << ct_carry->GetLevel() << "\n";

    return 0;
}
