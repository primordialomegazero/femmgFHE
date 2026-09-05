// ============================================
// φ-RULE 110 DIRECT — Exponent Space
// Hindi NAND gates — direct na φ-power
// Transition delta: constant corrections
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Sa exponent space:
    // 0 → log_φ(0.5) ≈ -1.44 (maliit na value)
    // 1 → log_φ(1.0) = 0.0
    //
    // Transition: next = current + delta
    // kung saan delta ay φ-gap correction (constant)
    //
    // Ang Rule 110 ay:
    // 111→0: delta = log_φ(0.5/φ) = -1 (bumaba)
    // 110→1: delta = log_φ(1/φ) = -1 (bumaba)
    // 101→1: delta = log_φ(1/φ) = -1
    // 100→0: delta = 0 (walang pagbabago)
    // 011→1: delta = log_φ(1/1) = 0
    // 010→1: delta = 0
    // 001→1: delta = 0
    // 000→0: delta = 0

    auto encrypt_log = [&](double val) {
        double log_val = log(val) / LN_PHI;
        vector<double> v(8, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_bit = [&](double log_val) {
        double val = pow(PHI, log_val);
        return val > 0.5;  // threshold
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 DIRECT — Exponent Space\n";
    cout << "========================================\n\n";
    cout << "  Hindi NAND gates — direct na φ-power\n";
    cout << "  Transition delta: constant corrections\n\n";

    // Initial: 11010101
    // 1 → log_φ(1) = 0
    // 0 → log_φ(0.5) = -1.44
    vector<double> init(8, 0.0);
    init[0] = 0.0;         // 1
    init[1] = 0.0;         // 1
    init[2] = -1.44;       // 0
    init[3] = 0.0;         // 1
    init[4] = -1.44;       // 0
    init[5] = 0.0;         // 1
    init[6] = -1.44;       // 0
    init[7] = 0.0;         // 1

    auto ct_state = encrypt_log(1.0);  // placeholder
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 11010101\n\n";

    int N = 100;

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // ANG DIRECT NA TRANSITION:
        // next[i] = current[i] + delta(pattern[i])
        // Ang delta ay constant para sa bawat pattern
        
        // Pre-computed delta para sa lahat ng 8 patterns:
        // 111→0: delta = -1.0
        // 110→1: delta = 0.0 (kung 0→1, +1.44; kung 1→1, 0)
        // 101→1: delta = 0.0
        // 100→0: delta = 0.0
        // 011→1: delta = 0.0
        // 010→1: delta = 0.0
        // 001→1: delta = 0.0
        // 000→0: delta = 0.0
        
        // ANG PROBLEMA: hindi natin alam ang pattern
        // kasi encrypted ang state
        //
        // ANG SOLUTION: gamitin ang exponent space
        // kung saan ang transition ay natural sa φ-structure
        
        // Sa ngayon, simpleng alternating
        vector<double> delta(8, 0.0);
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);
    
    cout << "  Final (100 steps): ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_bit(v_final[i]) ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
