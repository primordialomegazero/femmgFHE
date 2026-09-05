// ============================================
// φ-RULE 110 OPTIMIZED — Single Ciphertext
// 16 cells sa isang ciphertext
// Band polynomial na minimal ang EvalMult
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
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);
    const double EPSILON = pow(PHI, -6);
    const double LOWER = 5.0 * PHI - 7.0 - EPSILON;
    const double UPPER = 3.0 * PHI - 3.0 + EPSILON;

    cout << "========================================\n";
    cout << "  φ-RULE 110 OPTIMIZED — Single Ciphertext\n";
    cout << "========================================\n\n";
    cout << "  16 cells sa isang ciphertext\n";
    cout << "  Band: [" << LOWER << ", " << UPPER << "]\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(16);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 16; i++) out.push_back(res[i].real());
        return out;
    };

    // ============================================
    // Initial state: 2 cells lang ang 1 (sa gitna)
    // ============================================
    vector<double> init(16, V_ZERO);
    init[7] = V_ONE;
    init[8] = V_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 0000000110000000\n\n";

    // ============================================
    // Rule 110 evolution
    // ============================================
    int N = 10;
    
    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        // 1. Neighbor access — EvalRotate
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        // 2. Weighted sum: L + C + R
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // 3. Band polynomial: (sum - LOWER) × (UPPER - sum)
        auto ct_diff_lower = cc->EvalSub(ct_sum, LOWER);
        auto ct_diff_upper = cc->EvalSub(UPPER, ct_sum);
        auto ct_band = cc->EvalMult(ct_diff_lower, ct_diff_upper);
        
        // 4. Sign: 0.5 × band + 0.5
        auto ct_sign = cc->EvalMult(ct_band, 0.5);
        ct_sign = cc->EvalAdd(ct_sign, 0.5);
        
        // 5. State update
        ct_state = ct_sign;
        
        cout << "  Gen " << setw(2) << gen+1 << ": ";
        auto v = decrypt_state(ct_state);
        for (int i = 0; i < 16; i++) {
            int bit = (abs(v[i] - V_ONE) < abs(v[i] - V_ZERO)) ? 1 : 0;
            cout << bit;
        }
        cout << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
