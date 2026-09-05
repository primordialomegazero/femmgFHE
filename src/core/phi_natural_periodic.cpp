// ============================================
// φ-NATURAL PERIODIC — Universal Architecture
// Exponent na naka-mod sa φ — natural na reset
// Walang homomorphic modulo
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
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        // I-mod sa φ bago i-encrypt
        double n_mod = fmod(n, PHI);
        vector<double> v(1, n_mod);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-NATURAL PERIODIC — Universal Architecture\n";
    cout << "========================================\n\n";
    cout << "  Exponent naka-mod sa φ\n";
    cout << "  Natural na reset — walang homomorphic modulo\n\n";

    // ============================================
    // TEST 1: φ-periodic exponent
    // ============================================
    cout << "  TEST 1: φ-periodic exponent\n\n";
    cout << "  n | n mod φ | φ^(n mod φ)\n";
    cout << "  --|---------|------------\n";
    
    for (int n = 0; n <= 15; n++) {
        double n_mod = fmod((double)n, PHI);
        double val = pow(PHI, n_mod);
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << n_mod << " | "
             << setw(12) << val << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: 10K ops na may natural reset
    // ============================================
    cout << "  TEST 2: 10K ops na may natural reset\n\n";

    auto ct = encrypt_n(0.0);
    
    // Deltas para sa iba't ibang operations
    vector<double> deltas = {
        1.0,                        // ×φ
        -1.0,                       // ÷φ
        log(2.0) / LN_PHI,          // ×2
        -log(3.0) / LN_PHI,         // ÷3
        log(5.0) / LN_PHI,          // ×5
        -log(7.0) / LN_PHI,         // ÷7
        2.0,                        // +φ^(n+1) — mas malaking jump
        -3.0,                       // -φ^(n+2) — mas malaking jump
    };
    
    double expected_n = 0.0;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        double delta = deltas[i % deltas.size()];
        
        vector<double> d(1, abs(delta));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (delta >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        expected_n += delta;
        expected_n = fmod(expected_n, PHI);
        
        if (i % 2000 == 0) {
            double n_now = decrypt_n(ct);
            // I-mod sa φ
            double n_mod = fmod(n_now, PHI);
            cout << "    Step " << setw(4) << i << ": n=" << setw(10) << n_now
                 << ", n mod φ=" << setw(8) << n_mod
                 << ", value=" << setw(12) << pow(PHI, n_mod) << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct);
    double n_final_mod = fmod(n_final, PHI);
    double expected_final_mod = fmod(expected_n, PHI);
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (10000 * 1000.0) / time << "\n\n";
    cout << "  Final n: " << n_final << "\n";
    cout << "  Final n mod φ: " << n_final_mod << "\n";
    cout << "  Expected mod φ: " << expected_final_mod << "\n";
    cout << "  Match: " << (abs(n_final_mod - expected_final_mod) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Final value: " << pow(PHI, n_final_mod) << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
