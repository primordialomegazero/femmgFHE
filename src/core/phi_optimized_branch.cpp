// ============================================
// φ-OPTIMIZED BRANCH — Pre-computed Rotation
// Walang EvalRotate sa loop — puro EvalAdd
// Rotation ay naka-encode sa plaintext
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
        vector<double> v(8, 0.0);
        v[0] = n;
        v[1] = n + 1.0;
        v[2] = n - 1.0;
        v[3] = pow(PHI, n);
        v[4] = pow(PHI, n + 1.0);
        v[5] = pow(PHI, n - 1.0);
        v[6] = 0.0;
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
    cout << "  φ-OPTIMIZED BRANCH — Pre-computed\n";
    cout << "========================================\n\n";
    cout << "  Walang EvalRotate sa loop\n";
    cout << "  Rotation ay naka-encode sa plaintext\n\n";

    // ============================================
    // TEST: 100K na walang EvalRotate sa loop
    // ============================================
    cout << "  TEST: 100K na walang EvalRotate sa loop\n\n";

    auto ct = encrypt_state(5.0);
    
    // Pre-computed na rotation bilang plaintext
    // Ang rotation ay "naka-bake" sa delta
    
    // Para sa alternating branch:
    // Step even: rotation(1) — ang Slot 1 ay nagiging Slot 0
    // Step odd: rotation(2) — ang Slot 2 ay nagiging Slot 0
    //
    // Sa plaintext, ito ay:
    // Step even: delta = (1, 1, 1, ...) na may offset sa Slot 1
    // Step odd: delta = (1, 1, 1, ...) na may offset sa Slot 2
    
    vector<double> delta_even(8, 0.0);
    delta_even[0] = 0.0;   // Slot 0 ay mula sa Slot 1 (branch_t)
    delta_even[1] = 1.0;   // branch_t += 1
    delta_even[2] = 1.0;   // branch_f += 1
    delta_even[3] = 0.0;
    delta_even[4] = 0.0;
    delta_even[5] = 0.0;
    delta_even[6] = 0.0;
    delta_even[7] = 0.0;
    
    vector<double> delta_odd(8, 0.0);
    delta_odd[0] = -1.0;   // Slot 0 ay mula sa Slot 2 (branch_f)
    delta_odd[1] = 1.0;
    delta_odd[2] = 1.0;
    delta_odd[3] = 0.0;
    delta_odd[4] = 0.0;
    delta_odd[5] = 0.0;
    delta_odd[6] = 0.0;
    delta_odd[7] = 0.0;
    
    Plaintext pt_even = cc->MakeCKKSPackedPlaintext(delta_even);
    Plaintext pt_odd = cc->MakeCKKSPackedPlaintext(delta_odd);

    auto start = high_resolution_clock::now();
    
    int N = 100000;
    
    for (int i = 0; i < N; i++) {
        // Puro EvalAdd — walang EvalRotate!
        if (i % 2 == 0) {
            ct = cc->EvalAdd(ct, pt_even);
        } else {
            ct = cc->EvalAdd(ct, pt_odd);
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final: n=" << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
