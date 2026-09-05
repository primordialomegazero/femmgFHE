// ============================================
// φ-SPLIT CARRY — Pure FHE carry split
// 2φ^i = φ^(i+1) + φ^(i-2)
// I-split ang 2 sa dalawang 1s
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
    cout << "  φ-SPLIT CARRY — Pure FHE carry split\n";
    cout << "========================================\n\n";
    cout << "  2φ^i = φ^(i+1) + φ^(i-2)\n";
    cout << "  I-split ang 2 sa dalawang 1s\n\n";

    // ============================================
    // TEST: 5 + 3 = 8
    // ============================================
    cout << "  TEST: 5 + 3 = 8\n\n";

    // 5 = φ³(pos 10) + φ⁻¹(pos 6) + φ⁻⁴(pos 3)
    vector<double> bits_5(16, 0.0);
    bits_5[10] = 1.0;
    bits_5[6] = 1.0;
    bits_5[3] = 1.0;

    // 3 = φ²(pos 9) + φ⁻²(pos 5)
    vector<double> bits_3(16, 0.0);
    bits_3[9] = 1.0;
    bits_3[5] = 1.0;

    auto ct_5 = encrypt_bits(bits_5);
    auto ct_3 = encrypt_bits(bits_3);

    // Step 1: Component-wise addition
    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_bits = decrypt_bits(ct_sum);
    
    cout << "  Step 1 — Sum:\n  [";
    for (int i = 0; i < VEC_SIZE; i++) cout << setw(3) << sum_bits[i];
    cout << "]\n\n";

    // Step 2: I-extract ang '1' at '2' na positions
    // Ang '2' ay nangangahulugang carry
    // 2 = 1 + 1 — kaya ang 2 ay kailangang i-split
    
    // Sa FHE, i-rotate natin ang sum at i-subtract
    // para makuha ang carry-only na bahagi
    
    // I-rotate by +1: ang φ^(i-1) ay mag-a-align sa φ^i
    auto ct_rot_fwd = cc->EvalRotate(ct_sum, 1);
    
    // Ang carry detection: kung ang sum[i] >= 1 at sum[i-1] >= 1
    // Pero sa additive, ang sum[i] + sum[i-1] = 2 kapag parehong 1
    
    // I-rotate by -2 para makuha ang backward carry
    auto ct_rot_bwd = cc->EvalRotate(ct_sum, -2);
    
    // I-combine: ang 2φ^i ay nagiging φ^(i+1) + φ^(i-2)
    // Kaya: ct_sum + ct_rot_fwd + ct_rot_bwd
    // Pero dapat, ang 2 ay maging 0 muna...
    
    // Ang tamang split:
    // 1. I-rotate ang sum by +1 para sa forward carry
    // 2. I-rotate ang sum by -2 para sa backward carry
    // 3. I-add ang dalawang rotates sa sum
    // 4. Ang 2 sa sum ay magiging 2+1+1 = 4 — mali...
    
    // Mas maganda: i-subtract ang 2 muna, tapos i-add ang split
    // Pero paano i-subtract ang 2 nang walang comparison?
    
    // ANG TAMANG APPROACH:
    // Ang 2 ay natural na lumalabas kapag may overlap
    // Ang 2φ^i ay dapat maging φ^(i+1) + φ^(i-2)
    // Ibig sabihin: ang position i ay dapat maging 0,
    // at ang positions i+1 at i-2 ay dapat magdagdag ng 1
    
    // Ito ay maaaring gawin sa pamamagitan ng:
    // 1. I-detect ang 2 (rotate + add)
    // 2. I-subtract ang detection mula sa original
    // 3. I-add ang split sa result
    
    // Subukan natin ang mas simpleng approach:
    // Ang carry rule ay: φ^i + φ^(i-1) = φ^(i+1)
    // Kaya kung ang position i at i-1 ay parehong 1:
    // - I-zero ang positions i at i-1
    // - I-add ang 1 sa position i+1
    
    // Sa FHE, ito ay:
    // 1. I-rotate ang sum by 1 para makuha ang shifted
    // 2. Ang shifted + sum ay may 2 sa carry positions
    // 3. Ang 2 ay nangangahulugang: i-zero at i-carry
    
    // PAGSUBOK: simpleng iterative approach
    auto ct_result = ct_sum;
    
    cout << "  Iterative carry propagation:\n\n";
    
    for (int iter = 0; iter < 5; iter++) {
        // I-rotate by +1
        auto rot = cc->EvalRotate(ct_result, 1);
        
        // Ang carry: kung may 2, ang φ^i + φ^(i-1) = φ^(i+1)
        // I-add ang rotated, tapos i-subtract ang original
        ct_result = cc->EvalAdd(ct_result, rot);
        
        auto v = decrypt_bits(ct_result);
        cout << "    Iter " << iter << ": value = " << value_from_bits(v) << " | [";
        for (int i = 0; i < VEC_SIZE; i++) {
            if (abs(v[i]) > 0.5) cout << v[i];
            else cout << ".";
        }
        cout << "]\n";
    }
    
    cout << "\n  Level: " << ct_result->GetLevel() << "\n";

    return 0;
}
