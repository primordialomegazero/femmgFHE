// ============================================
// φ-IRRATIONAL BRANCH — Quasi-Random Selection
// Ang irrationality ng φ ay nagbibigay ng
// natural na data-dependent na branch
// Hindi alternating, hindi predictable
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
        v[1] = n + 1.0;   // branch_t
        v[2] = n - 1.0;   // branch_f
        v[3] = n + 0.5;
        v[4] = n - 0.3;
        v[5] = fmod(n * PHI, 1.0);      // fractional part ng n×φ — IRRATIONAL
        v[6] = fmod(n * PHI * PHI, 1.0); // fractional part ng n×φ²
        v[7] = floor(n * PHI);           // Beatty sequence — IRRATIONAL
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
    cout << "  φ-IRRATIONAL BRANCH — Quasi-Random\n";
    cout << "========================================\n\n";
    cout << "  Ang irrationality ng φ ay nagbibigay ng\n";
    cout << "  natural na data-dependent branch\n\n";

    // ============================================
    // TEST 1: Ang irrational na pattern
    // ============================================
    cout << "  TEST 1: Irrational na pattern\n\n";
    cout << "  n | frac(n×φ) | floor(n×φ) | parity\n";
    cout << "  --|------------|------------|--------\n";
    
    for (int n = 0; n <= 15; n++) {
        double frac = fmod(n * PHI, 1.0);
        double beatty = floor(n * PHI);
        int parity = (int)beatty % 2;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << frac << " | "
             << setw(10) << beatty << " | "
             << setw(6) << parity << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: Irrational branch sa FHE
    // ============================================
    cout << "  TEST 2: Irrational branch sa FHE\n\n";

    auto ct = encrypt_state(3.0);
    
    cout << "    Initial state (n=3):\n";
    auto v_init = decrypt_state(ct);
    cout << "    frac(n×φ) = " << v_init[5] << "\n";
    cout << "    frac(n×φ²) = " << v_init[6] << "\n";
    cout << "    floor(n×φ) = " << v_init[7] << "\n\n";

    // Ang branch selection ay mula sa parity ng floor(n×φ)
    // Na IRRATIONAL — hindi alternating
    // Natural na quasi-random na pagpili
    
    auto start = high_resolution_clock::now();
    
    vector<double> d(8, 1.0);
    d[5] = PHI;    // frac update: n×φ
    d[6] = PHI * PHI;  // frac update: n×φ²
    d[7] = PHI;    // Beatty update
    Plaintext pt_d = cc->MakeCKKSPackedPlaintext(d);
    
    for (int i = 0; i < 1000; i++) {
        ct = cc->EvalAdd(ct, pt_d);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    After 1000 steps:\n";
    cout << "    n = " << v_final[0] << "\n";
    cout << "    frac(n×φ) = " << v_final[5] << "\n";
    cout << "    floor(n×φ) = " << v_final[7] << "\n";
    cout << "    Parity = " << ((int)v_final[7] % 2) << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
