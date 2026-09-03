// ============================================
// φ-PURE TRANSITION V3
// Tamang computation ng bridge sa FHE
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

    cout << "=== φ-PURE TRANSITION V3 ===\n\n";

    // ============================================
    // TEST: (F_5 + F_3) × F_4 = 21
    // ============================================
    cout << "--- TEST: (F_5 + F_3) × F_4 = 21 ---\n\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);

        // Addition sa normal space
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        double q_sum = v_add[1];
        double r_sum = v_add[0];
        
        cout << "  q_sum = " << q_sum << "\n";
        cout << "  r_sum = " << r_sum << "\n";
        
        // Tamang bridge computation
        double log_q = log_q_table[(int)q_sum];
        double corr = log(1.0 + r_sum / (q_sum * PHI)) / LN_PHI;
        double log_F_pure = log_q + 1.0 + corr;
        
        cout << "  log_q = " << log_q << "\n";
        cout << "  corr = " << corr << "\n";
        cout << "  log_F_pure = " << log_F_pure << "\n";
        cout << "  log_F_exact = " << log(7.0) / LN_PHI << "\n";
        cout << "  Match: " << (abs(log_F_pure - log(7.0)/LN_PHI) < 0.001 ? "✅" : "❌") << "\n\n";

        // Multiplication: 7 × 3
        vector<double> v_trans(4, 0.0);
        v_trans[2] = log_F_pure;
        Plaintext pt_trans = cc->MakeCKKSPackedPlaintext(v_trans);
        auto ct_trans = cc->Encrypt(keyPair.publicKey, pt_trans);
        
        vector<double> v_c(4, 0.0);
        v_c[2] = log(3.0) / LN_PHI;
        Plaintext pt_c = cc->MakeCKKSPackedPlaintext(v_c);
        auto ct_c = cc->Encrypt(keyPair.publicKey, pt_c);
        
        auto ct_result = cc->EvalAdd(ct_trans, ct_c);
        auto v_result = decrypt_full(ct_result);
        double result = pow(PHI, v_result[2]);
        
        cout << "  Result: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 0.5 ? "✅" : "❌") << "\n";
    }

    return 0;
}
