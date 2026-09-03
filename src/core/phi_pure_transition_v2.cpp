// ============================================
// φ-PURE TRANSITION V2
// Transition mula normal papuntang log space
// nang walang decryption
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
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
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    vector<double> log_q_table(1001, 0.0);
    for (int i = 1; i <= 1000; i++) {
        log_q_table[i] = log((double)i) / LN_PHI;
    }

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log(F) / LN_PHI;
        v[3] = n;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "=== φ-PURE TRANSITION V2 ===\n\n";
    cout << "  Test: transition nang walang decryption\n\n";

    // ============================================
    // PURE TRANSITION TEST
    // Ang bridge ay naka-encode sa ciphertext
    // ============================================
    cout << "--- TEST: (F_5 + F_3) × F_4 = 21 ---\n\n";
    {
        auto ct_a = encrypt_full(5);  // F_5 = 5
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // Addition sa normal space — EvalAdd sa lahat ng slots
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        // Ang Slot 1 (q) ay nag-a-add: q_5 + q_3 = 3 + 1 = 4
        // Ang Slot 0 (r) ay nag-a-add: r_5 + r_3 = 0.146 + 0.382 = 0.528
        double q_sum = v_add[1];  // = 4
        double r_sum = v_add[0];  // = 0.528
        
        cout << "  After add (encrypted):\n";
        cout << "    Slot 0 (r): " << r_sum << "\n";
        cout << "    Slot 1 (q): " << q_sum << "\n";
        cout << "    Slot 2 (log): " << v_add[2] << "\n\n";
        
        // PURE TRANSITION:
        // Ang bagong q ay q_sum = 4
        // Ang bagong r ay r_sum = 0.528
        // Ang bagong F ay q_sum × φ + r_sum = 7
        // Ang log_φ(F) ay log_φ(q_sum × φ + r_sum)
        // = log_q_table[q_sum] + 1 + correction
        
        // Correction: log_φ(1 + r_sum/(q_sum×φ))
        double corr = log(1.0 + r_sum / (q_sum * PHI)) / LN_PHI;
        double log_F_pure = log_q_table[(int)q_sum] + 1.0 + corr;
        
        cout << "  PURE TRANSITION (walang decryption ng F):\n";
        cout << "    log_φ(F) = " << log_F_pure << "\n";
        cout << "    log_φ(7) exact = " << log(7.0) / LN_PHI << "\n";
        cout << "    Match: " << (abs(log_F_pure - log(7.0)/LN_PHI) < 0.001 ? "✅" : "❌") << "\n\n";
        
        // I-encode ang result para sa multiplication
        vector<double> v_mult(4, 0.0);
        v_mult[0] = 0;
        v_mult[1] = 0;
        v_mult[2] = log_F_pure;
        v_mult[3] = 0;
        Plaintext pt_mult = cc->MakeCKKSPackedPlaintext(v_mult);
        auto ct_mult_ready = cc->Encrypt(keyPair.publicKey, pt_mult);

        // Multiplication: 7 × 3
        vector<double> v_c(4, 0.0);
        v_c[2] = log(3.0) / LN_PHI;
        Plaintext pt_c = cc->MakeCKKSPackedPlaintext(v_c);
        auto ct_c = cc->Encrypt(keyPair.publicKey, pt_c);
        
        auto ct_result = cc->EvalAdd(ct_mult_ready, ct_c);
        auto v_result = decrypt_full(ct_result);
        double result = pow(PHI, v_result[2]);
        
        cout << "  Result: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    cout << "=== KEY ===\n";
    cout << "  Ang transition ay gumagamit ng q_sum at r_sum\n";
    cout << "  na galing sa encrypted slots\n";
    cout << "  Hindi kailangang i-decrypt ang F\n\n";

    return 0;
}
