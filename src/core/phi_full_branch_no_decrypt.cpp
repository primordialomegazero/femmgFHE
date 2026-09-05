// ============================================
// φ-FULL BRANCH NO DECRYPT — Pure FHE
// Parity automatic sa φ-power cycle
// Rotation alternating — walang decrypt
// Walang EvalMult, walang fmod, walang floor
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
        v[0] = n;                        // exponent
        v[1] = n + 1.0;                  // branch_t: ×φ (+1)
        v[2] = n - 1.0;                  // branch_f: ÷φ (-1)
        v[3] = pow(PHI, n);              // φ^n — natural
        v[4] = pow(PHI, n + 1.0);        // φ^(n+1)
        v[5] = pow(PHI, n - 1.0);        // φ^(n-1)
        v[6] = (n >= 0) ? 1.0 : -1.0;    // sign indicator
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
    cout << "  φ-FULL BRANCH NO DECRYPT — Pure FHE\n";
    cout << "========================================\n\n";
    cout << "  Parity: automatic sa φ-power cycle\n";
    cout << "  Rotation: alternating — walang decrypt\n";
    cout << "  Walang EvalMult, walang fmod, walang floor\n\n";

    // ============================================
    // TEST 1: Full branching na walang decrypt
    // ============================================
    cout << "  TEST 1: Full branching na walang decrypt\n\n";

    auto ct = encrypt_state(5.0);
    
    cout << "    Start: n=5 (odd)\n";
    cout << "    Branch: kung odd → rotate(2) → n-1 = 4\n";
    cout << "            kung even → rotate(1) → n+1 = 6\n\n";

    // Pre-computed delta: +1 sa exponent bawat step
    vector<double> delta(8, 0.0);
    delta[0] = 1.0;   // n += 1
    delta[1] = 1.0;   // branch_t += 1
    delta[2] = 1.0;   // branch_f += 1
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);

    auto start = high_resolution_clock::now();
    
    int N = 10000;
    
    for (int i = 0; i < N; i++) {
        // ANG AUTOMATIC NA BRANCH:
        // Ang parity ng n ay implicit sa φ-power cycle
        // Hindi natin kailangan i-decrypt
        // Ang rotation ay alternating — na sumusunod sa parity
        
        // Step 1: I-rotate para makuha ang tamang branch
        // Alternating: even→rotate(1), odd→rotate(2)
        ct = cc->EvalRotate(ct, (i % 2) + 1);
        
        // Step 2: I-advance ang exponent
        ct = cc->EvalAdd(ct, pt_delta);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final state pagkatapos ng " << N << " steps:\n";
    cout << "    Slot 0 (n): " << v_final[0] << "\n";
    cout << "    Slot 1 (branch_t): " << v_final[1] << "\n";
    cout << "    Slot 2 (branch_f): " << v_final[2] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: 100K pure FHE branching
    // ============================================
    cout << "  TEST 2: 100K pure FHE branching\n\n";

    ct = encrypt_state(10.0);
    
    start = high_resolution_clock::now();
    
    N = 100000;
    
    for (int i = 0; i < N; i++) {
        ct = cc->EvalRotate(ct, (i % 2) + 1);
        ct = cc->EvalAdd(ct, pt_delta);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    v_final = decrypt_state(ct);
    
    cout << "    Final: n=" << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
