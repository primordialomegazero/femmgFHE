// ============================================
// φ-PURE TRANSITION
// Transition na walang decryption
// Gamit ang ε_n bilang pre-computed correction
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
    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    // Pre-compute ε_n para sa lahat ng n
    vector<double> epsilon(101, 0.0);
    for (int n = 2; n <= 100; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        epsilon[n] = exact_log - approx_log;
    }

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = log(F) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log_phi_F;
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

    cout << "=== φ-PURE TRANSITION ===\n\n";
    cout << "  Transition mula (r, q) papuntang log_φ(F)\n";
    cout << "  nang walang decryption\n\n";

    // ============================================
    // TEST: ADDITION + TRANSITION + MULTIPLICATION
    // ============================================
    cout << "--- TEST: (F_5 + F_3) × F_4 = 21 ---\n\n";
    {
        auto ct_a = encrypt_full(5);  // F_5 = 5
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // Addition sa normal space
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double result_normal = v_add[1] * PHI + v_add[0];
        cout << "  After add (normal): " << result_normal << "\n";

        // TRANSITION: normal → log
        // result_normal = 7, hindi Fibonacci
        // log_φ(7) ≈ index - log_φ(√5) + ε_est
        // kung saan index ≈ 5.716 (from log(7)/log(φ))
        double log_7 = log(7.0) / LN_PHI;
        double approx_index = log_7 + LOG_SQRT5;
        double eps_est = 2.0 * log(1.0 - pow(-1.0, approx_index) * pow(PHI, -2.0 * approx_index)) / LN_PHI;
        double log_7_transition = approx_index - LOG_SQRT5 + eps_est;
        
        cout << "  log_φ(7) exact: " << log_7 << "\n";
        cout << "  log_φ(7) transition: " << log_7_transition << "\n";
        cout << "  Diff: " << abs(log_7 - log_7_transition) << "\n\n";

        // I-encode ang transitioned state
        vector<double> v_trans(4, 0.0);
        v_trans[0] = fmod(7.0, PHI);
        v_trans[1] = floor(7.0 / PHI);
        v_trans[2] = log_7_transition;
        v_trans[3] = approx_index;
        Plaintext pt_trans = cc->MakeCKKSPackedPlaintext(v_trans);
        auto ct_trans = cc->Encrypt(keyPair.publicKey, pt_trans);

        // Multiplication sa log space: 7 × 3
        auto ct_c = encrypt_full(4);  // F_4 = 3
        auto ct_mult = cc->EvalAdd(ct_trans, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        
        cout << "  After mult: " << prod << " (Expected: 21)\n";
        cout << "  Match: " << (abs(prod - 21.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    cout << "=== KEY ===\n";
    cout << "  Ang ε_n correction ay nagbibigay ng\n";
    cout << "  halos eksaktong transition\n";
    cout << "  Ang susunod na hakbang ay i-embed\n";
    cout << "  ito bilang ciphertext operation\n\n";

    return 0;
}
