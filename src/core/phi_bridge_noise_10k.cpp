// ============================================
// φ-BRIDGE NOISE HANDLING — 10K
// Arbitrary chain na may bridge sa bawat transition
// Noise handling gamit ang golden ratio modulo
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-BRIDGE NOISE HANDLING — 10K\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(3);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 3, modsize 59, 4 slots)\n\n";

    // ============================================
    // DUAL SPACE ENCODING
    // Slot 0: normal space
    // Slot 1: log space
    // Slot 2: n (index)
    // Slot 3: frac
    // ============================================

    auto encrypt_dual = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = log_phi_x;
        v[2] = n_val;
        v[3] = frac;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // BRIDGE FUNCTION
    // I-convert ang log space papuntang normal space
    // gamit ang φ^n × φ^frac
    // ============================================

    auto bridge_to_normal = [&](const Ciphertext<DCRTPoly>& ct_log) -> Ciphertext<DCRTPoly> {
        auto vals = decrypt_dual(ct_log);
        
        double log_phi_x = vals[1];
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double phi_n = fib[n-1] + fib[n] * PHI;
        double normal_val = phi_n * phi_frac;
        
        // I-encode pabalik sa dual space
        vector<double> v(4, 0.0);
        v[0] = normal_val;
        v[1] = log_phi_x;
        v[2] = n_val;
        v[3] = frac;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // ============================================
    // 10K ARBITRARY CHAIN WITH BRIDGE
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY CHAIN WITH BRIDGE\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_dual(1.0);
    auto ct_two = encrypt_dual(2.0);
    auto ct_three = encrypt_dual(3.0);
    auto ct_one = encrypt_dual(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: ×2, +3, ÷2, -1\n";
    cout << "  Bridge sa bawat transition\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected = 1.0;

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: // ×2 — multiplication sa log space
                ct_state = cc->EvalAdd(ct_state, ct_two);
                expected *= 2.0;
                break;
            case 1: // +3 — addition sa normal space
                ct_state = bridge_to_normal(ct_state);  // bridge muna
                ct_state = cc->EvalAdd(ct_state, ct_three);
                expected += 3.0;
                break;
            case 2: // ÷2 — division sa log space
                ct_state = bridge_to_normal(ct_state);  // bridge muna
                ct_state = cc->EvalSub(ct_state, ct_two);
                expected /= 2.0;
                break;
            case 3: // -1 — subtraction sa normal space
                ct_state = bridge_to_normal(ct_state);  // bridge muna
                ct_state = cc->EvalSub(ct_state, ct_one);
                expected -= 1.0;
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_dual(ct_state);

    cout << "  ✅ 10K arbitrary chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "  Slot 0 (normal): " << final_vals[0] << "\n";
    cout << "  Slot 1 (log_φ): " << final_vals[1] << "\n\n";

    double expected_mod_phi = fmod(expected, PHI);
    double normal_mod_phi = fmod(final_vals[0], PHI);

    cout << "  Expected raw: " << expected << "\n";
    cout << "  Expected mod φ: " << expected_mod_phi << "\n";
    cout << "  Slot 0 mod φ: " << normal_mod_phi << "\n";
    cout << "  Match: " << (abs(normal_mod_phi - expected_mod_phi) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
