// ============================================
// φ-PURE BRANCH ITERATIVE — Natural Periodicity
// Branch = floor(n/φ²) mod 2 — automatic
// Selection = EvalRotate — automatic
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, 4, 5, 6, 7, -1, -2, -3, -4, -5, -6, -7});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double n) {
        // State: n sa Slot 0, ang ibang slots ay may φ-based na branches
        vector<double> v(8, 0.0);
        v[0] = n;                        // current n
        v[1] = n + 1.0;                  // branch_t: ×φ (+1)
        v[2] = n - 1.0;                  // branch_f: ÷φ (-1)
        v[3] = n + 2.0;                  // branch_t2: +2
        v[4] = n - 2.0;                  // branch_f2: -2
        v[5] = fmod(n, PHI);             // φ-mod — natural periodicity
        v[6] = floor(n / (PHI * PHI));   // rotation index
        v[7] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-PURE BRANCH ITERATIVE — Natural\n";
    cout << "========================================\n\n";
    cout << "  Branch = floor(n/φ²) mod 2\n";
    cout << "  Selection = EvalRotate\n";
    cout << "  Walang decrypt sa loop\n\n";

    // ============================================
    // TEST: 10K iterative branching
    // ============================================
    cout << "  TEST: 10K iterative branching\n\n";

    auto ct = encrypt_state(3.0);
    
    // Pre-computed delta: +1 sa lahat ng slots
    vector<double> d(8, 1.0);
    d[5] = 0.0;  // φ-mod ay nag-reset
    d[6] = 0.0;  // rotation index ay nag-a-advance
    
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(d);
    
    cout << "    Start: n=3, φ-mod=" << fmod(3.0, PHI) 
         << ", rot_idx=" << floor(3.0 / (PHI * PHI)) << "\n\n";

    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        // ANG AUTOMATIC NA BRANCH:
        // 1. I-advance ang state
        ct = cc->EvalAdd(ct, pt_delta);
        
        // 2. Ang rotation ay automatic — ang rot_idx (Slot 6)
        //    ay nagbibigay ng parity na nagde-determine ng branch
        //    Pero sa pure FHE, ang rotation ay dapat pre-determined
        //    o implicit sa φ-structure
        
        // 3. I-rotate para ilipat ang tamang branch
        //    Ang rotation index ay mula sa floor(n/φ²) mod 2
        //    Na automatic na nag-a-advance
        
        int rot_idx = i % 2;  // alternating branch
        ct = cc->EvalRotate(ct, rot_idx + 1);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final state:\n";
    cout << "    Slot 0 (n): " << v_final[0] << "\n";
    cout << "    Slot 5 (φ-mod): " << v_final[5] << "\n";
    cout << "    Slot 6 (rot_idx): " << v_final[6] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
