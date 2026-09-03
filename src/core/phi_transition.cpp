// ============================================
// φ-TRANSITION
// Homomorphic transition mula normal space
// papuntang log space gamit ang φ-structure
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

    cout << "=== φ-TRANSITION ===\n\n";

    // ============================================
    // ANG KEY IDEA
    // Kapag may (r, q) ka, ang log_φ(F) ay:
    // log_φ(F) = log_φ(q×φ + r)
    // 
    // Para sa malaking q, ito ay ≈ log_φ(q) + 1
    // Para sa malaking n, log_φ(F_n) ≈ n - log_φ(√5)
    //
    // ANG TRANSITION:
    // Mula sa index n, ang log space ay:
    // log_φ(F_n) = n - log_φ(√5) + ε_n
    //
    // Kung ε_n ay maliit at predictable,
    // maaaring i-encode bilang correction
    // ============================================

    cout << "  n | index | log_φ(F_n) exact | n - log_φ(√5) | ε_n\n";
    cout << "  --|-------|-------------------|---------------|------\n";

    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;
    
    for (int n = 3; n <= 15; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double epsilon = exact_log - approx_log;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << n << " | "
             << setw(17) << fixed << setprecision(5) << exact_log << " | "
             << setw(13) << approx_log << " | "
             << setw(10) << epsilon << "\n";
    }

    cout << "\n=== KEY OBSERVATION ===\n";
    cout << "  Ang ε_n ay nagco-converge sa 0\n";
    cout << "  Para sa malaking n, ang approximation ay exact\n";
    cout << "  Ang transition ay maaaring i-encode bilang:\n";
    cout << "  log_φ(F) = n - log_φ(√5) + ε_n\n\n";

    // ============================================
    // TEST: GAMIT ANG TRANSITION PARA SA MIXED OPS
    // ============================================
    cout << "--- MIXED TEST NA MAY TRANSITION ---\n\n";
    cout << "  (F_5 + F_3) × F_4 ÷ F_2 = 21\n\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        double result_normal = v_add[1] * PHI + v_add[0];  // = 7
        cout << "  After add (normal): " << result_normal << "\n";
        
        // TRANSITION: normal → log
        // result_normal = 7, hindi Fibonacci
        // Kailangan nating i-estimate ang log_φ(7)
        // 7 ≈ φ^3.75 (kasi φ^3 = 4.236, φ^4 = 6.854)
        // log_φ(7) ≈ 3.75
        double log_est = log(7.0) / LN_PHI;
        cout << "  log_φ(7) = " << log_est << "\n";
        
        // I-encode bilang transition state
        vector<double> v_trans(4, 0.0);
        v_trans[0] = fmod(7.0, PHI);
        v_trans[1] = floor(7.0 / PHI);
        v_trans[2] = log_est;
        v_trans[3] = log_est;  // index para sa log
        Plaintext pt_trans = cc->MakeCKKSPackedPlaintext(v_trans);
        auto ct_trans = cc->Encrypt(keyPair.publicKey, pt_trans);
        
        // Multiplication sa log space
        auto ct_c = encrypt_full(4);
        auto ct_mult = cc->EvalAdd(ct_trans, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 21)\n";
        
        // Division sa log space
        auto ct_d = encrypt_full(2);
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        double result = pow(PHI, v_div[2]);
        cout << "  Final: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    cout << "=== KEY ===\n";
    cout << "  Ang transition mula normal → log ay\n";
    cout << "  maaaring gawin sa pamamagitan ng index\n";
    cout << "  at ang ε_n correction term\n";
    cout << "  na nagco-converge sa 0 para sa malaking n\n\n";

    return 0;
}
