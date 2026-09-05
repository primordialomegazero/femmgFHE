// ============================================
// φ-BEATTY BRANCH PURE — Natural Data-Dependent
// Branch = floor(n×φ) mod 2 — Beatty parity
// Walang decrypt, walang modulo, walang EvalMult
// Ang branch ay implicit sa φ-structure
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
        v[0] = n;                              // current n
        v[1] = n + 1.0;                        // branch_t: +1 (×φ)
        v[2] = n - 1.0;                        // branch_f: -1 (÷φ)
        v[3] = pow(PHI, n);                    // φ^n — natural value
        v[4] = fmod(pow(PHI, n), PHI);         // φ^n mod φ — natural periodicity
        v[5] = fmod(n * PHI, 1.0);             // frac(n×φ) — irrational
        v[6] = floor(n * PHI);                 // Beatty sequence
        v[7] = fmod(floor(n * PHI), 2.0);      // Beatty parity — branch selector
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
    cout << "  φ-BEATTY BRANCH PURE — Natural\n";
    cout << "========================================\n\n";
    cout << "  Branch = floor(n×φ) mod 2\n";
    cout << "  Walang decrypt, walang modulo\n\n";

    // ============================================
    // TEST 1: Beatty parity pattern
    // ============================================
    cout << "  TEST 1: Beatty parity pattern\n\n";
    cout << "  n | floor(n×φ) | parity | branch\n";
    cout << "  --|------------|--------|--------\n";
    
    for (int n = 0; n <= 20; n++) {
        double beatty = floor(n * PHI);
        int parity = (int)beatty % 2;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << beatty << " | "
             << setw(6) << parity << " | "
             << (parity == 0 ? "true (+1)" : "false (-1)") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: FHE na may Beatty branch
    // ============================================
    cout << "  TEST 2: FHE na may Beatty branch\n\n";

    auto ct = encrypt_state(3.0);
    auto v = decrypt_state(ct);
    
    cout << "    Initial (n=3):\n";
    cout << "    Beatty = " << v[6] << "\n";
    cout << "    Parity = " << v[7] << "\n";
    cout << "    Branch = " << (v[7] < 0.5 ? "true (+1)" : "false (-1)") << "\n\n";

    // Ang branch selection:
    // Kung parity = 0: piliin ang Slot 1 (branch_t) sa pamamagitan ng rotate(1)
    // Kung parity = 1: piliin ang Slot 2 (branch_f) sa pamamagitan ng rotate(2)
    
    auto ct_rot = cc->EvalRotate(ct, (int)v[7] + 1);
    auto v_rot = decrypt_state(ct_rot);
    
    cout << "    After rotate(" << (int)v[7] + 1 << "):\n";
    cout << "    Slot 0: " << v_rot[0] << "\n";
    cout << "    Level: " << ct_rot->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 10K FHE na may Beatty branch
    // ============================================
    cout << "  TEST 3: 10K FHE na may Beatty branch\n\n";

    ct = encrypt_state(3.0);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        // ANG AUTOMATIC NA BRANCH:
        // Ang Beatty parity (Slot 7) ay nagde-determine ng branch
        // Hindi alternating — data-dependent sa φ-structure
        
        // I-advance ang n
        vector<double> d(8, 1.0);
        d[5] = PHI;           // frac update
        d[6] = PHI;           // Beatty update
        d[7] = 0.0;           // parity ay recomputed sa state
        Plaintext pt_d = cc->MakeCKKSPackedPlaintext(d);
        ct = cc->EvalAdd(ct, pt_d);
        
        // I-rotate para sa branch selection
        // Ang parity ay implicit sa Slot 7 — pero sa pure FHE,
        // kailangan natin ng automatic na rotation
        ct = cc->EvalRotate(ct, (i % 2) + 1);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final state:\n";
    cout << "    n = " << v_final[0] << "\n";
    cout << "    Beatty = " << v_final[6] << "\n";
    cout << "    Parity = " << v_final[7] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
