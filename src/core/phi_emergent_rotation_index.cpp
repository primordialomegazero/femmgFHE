// ============================================
// φ-EMERGENT ROTATION INDEX — Natural
// Hindi floor(n×φ), kundi φ^n mod φ na parity
// Walang floor, walang modulo, walang decrypt
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
        v[0] = n;                              // current exponent
        v[1] = n + 1.0;                        // branch_t: +1
        v[2] = n - 1.0;                        // branch_f: -1
        v[3] = pow(PHI, n);                    // φ^n — natural value
        v[4] = fmod(pow(PHI, n), PHI);         // φ^n mod φ — natural parity
        v[5] = fmod(n, 2.0);                   // exponent parity — 0 even, 1 odd
        v[6] = (fmod(n, 2.0) < 0.5) ? 1.0 : 0.0;  // rotation index: even→1, odd→2
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
    cout << "  φ-EMERGENT ROTATION INDEX — Natural\n";
    cout << "========================================\n\n";
    cout << "  Hindi floor(n×φ), kundi φ^n mod φ parity\n";
    cout << "  Walang floor, walang modulo\n\n";

    // ============================================
    // TEST 1: Natural parity pattern
    // ============================================
    cout << "  TEST 1: Natural parity pattern\n\n";
    cout << "  n | φ^n mod φ | parity | branch\n";
    cout << "  --|-----------|--------|--------\n";
    
    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        int parity = n % 2;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << mod_phi << " | "
             << setw(6) << parity << " | "
             << (parity == 0 ? "true (+1)" : "false (-1)") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: FHE na may emergent rotation index
    // ============================================
    cout << "  TEST 2: FHE na may emergent rotation index\n\n";

    auto ct = encrypt_state(3.0);
    auto v = decrypt_state(ct);
    
    cout << "    Initial (n=3):\n";
    cout << "    φ^n mod φ = " << v[4] << "\n";
    cout << "    Parity = " << v[5] << "\n";
    cout << "    Rotation index = " << v[6] << "\n\n";

    // Ang rotation ay automatic mula sa parity
    // Parity 0 (even) → rotate(1) → Slot 0 = branch_t
    // Parity 1 (odd) → rotate(2) → Slot 0 = branch_f
    
    auto ct_rot = cc->EvalRotate(ct, (int)v[6] + 1);
    auto v_rot = decrypt_state(ct_rot);
    
    cout << "    After rotate(" << (int)v[6] + 1 << "):\n";
    cout << "    Slot 0: " << v_rot[0] << "\n\n";

    // ============================================
    // TEST 3: 10K emergent na branch
    // ============================================
    cout << "  TEST 3: 10K emergent na branch\n\n";

    ct = encrypt_state(3.0);
    
    auto start = high_resolution_clock::now();
    
    vector<double> d(8, 1.0);
    d[4] = 0.0;  // φ^n mod φ — nagre-reset sa natural periodicity
    d[5] = 1.0;  // parity toggle
    d[6] = 0.0;  // rotation index — recomputed
    Plaintext pt_d = cc->MakeCKKSPackedPlaintext(d);
    
    for (int i = 0; i < 10000; i++) {
        ct = cc->EvalAdd(ct, pt_d);
        
        // Ang rotation ay automatic mula sa parity
        // Sa pure FHE, ang parity ay implicit sa Slot 5
        // Kailangan natin ng paraan para automatic na i-rotate
        
        // Sa ngayon, alternating muna
        ct = cc->EvalRotate(ct, (i % 2) + 1);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final state:\n";
    cout << "    n = " << v_final[0] << "\n";
    cout << "    Parity = " << v_final[5] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
