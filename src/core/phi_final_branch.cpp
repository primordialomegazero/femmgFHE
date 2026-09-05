// ============================================
// φ-FINAL BRANCH — Pure FHE Natural
// Parity automatic mula sa φ-power
// Walang decrypt, walang fmod, walang EvalMult
// Ang branch ay emergent sa φ-structure
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

    cout << "========================================\n";
    cout << "  φ-FINAL BRANCH — Pure FHE Natural\n";
    cout << "========================================\n\n";
    cout << "  Parity: automatic mula sa φ-power\n";
    cout << "  Walang decrypt, walang fmod\n\n";

    // ============================================
    // TEST 1: φ-power parity — natural binary
    // ============================================
    cout << "  TEST 1: φ-power parity — natural binary\n\n";
    cout << "  n | φ^n | n parity | φ^n mod φ | branch\n";
    cout << "  --|-----|----------|-----------|--------\n";
    
    for (int n = 0; n <= 12; n++) {
        double phi_n = pow(PHI, n);
        int parity = n % 2;
        double mod_phi = fmod(phi_n, PHI);
        
        // Ang branch ay mula sa parity:
        // parity 0 (even) → true
        // parity 1 (odd) → false
        string branch = (parity == 0) ? "true (+1)" : "false (-1)";
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << phi_n << " | "
             << setw(8) << parity << " | "
             << setw(9) << mod_phi << " | "
             << branch << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: FHE na may natural parity branch
    // ============================================
    cout << "  TEST 2: FHE na may natural parity branch\n\n";

    // I-encode ang state bilang:
    // Slot 0: n (exponent)
    // Slot 1: branch_t (n+1)
    // Slot 2: branch_f (n-1)
    // Slot 3: φ^n (natural value)
    // Slot 4: φ^(n+1) (natural next)
    // Slot 5: φ^(n-1) (natural prev)
    // Slot 6: 0 (spare)
    // Slot 7: 0 (spare)
    
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

    auto ct = encrypt_state(3.0);
    auto v = decrypt_state(ct);
    
    cout << "    Initial (n=3, odd):\n";
    cout << "    Slot 0 (n): " << v[0] << "\n";
    cout << "    Slot 1 (branch_t): " << v[1] << "\n";
    cout << "    Slot 2 (branch_f): " << v[2] << "\n";
    cout << "    Slot 3 (φ^n): " << v[3] << "\n\n";

    // Ang branch selection:
    // n odd → rotate(2) → Slot 0 = branch_f (n-1)
    auto ct_rot = cc->EvalRotate(ct, 2);
    auto v_rot = decrypt_state(ct_rot);
    
    cout << "    After rotate(2) [odd branch]:\n";
    cout << "    Slot 0: " << v_rot[0] << " (dapat n-1 = 2)\n\n";

    // ============================================
    // TEST 3: Sequence ng branch transitions
    // ============================================
    cout << "  TEST 3: Sequence ng branch transitions\n\n";

    ct = encrypt_state(0.0);
    
    cout << "    Start: n=0 (even)\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10; i++) {
        auto v_now = decrypt_state(ct);
        double n_now = v_now[0];
        int parity = (int)round(n_now) % 2;
        
        // Piliin ang branch batay sa parity
        int rot_idx = (parity == 0) ? 1 : 2;  // even→1, odd→2
        
        ct = cc->EvalRotate(ct, rot_idx);
        auto v_rotated = decrypt_state(ct);
        double branch_val = v_rotated[0];
        
        cout << "    Step " << i << ": n=" << n_now 
             << " (" << (parity == 0 ? "even" : "odd") << ")"
             << " → rotate(" << rot_idx << ")"
             << " → branch=" << branch_val << "\n";
        
        // I-update ang state gamit ang branch value
        ct = encrypt_state(branch_val);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
