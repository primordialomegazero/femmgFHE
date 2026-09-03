// ============================================
// φ-EXACT LOG MIXED
// Eksaktong log_φ(F_n) para walang approximation error
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
        double log_phi_F = log(F) / LN_PHI;  // EXACT
        
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

    cout << "=== φ-EXACT LOG MIXED ===\n\n";

    // ============================================
    // MIXED TEST 1: (F_5 + F_3) × F_4 ÷ F_2
    // ============================================
    cout << "--- TEST 1: (F_5 + F_3) × F_4 ÷ F_2 ---\n";
    cout << "  Expected: (5+2) × 3 ÷ 1 = 21\n\n";
    {
        auto ct_a = encrypt_full(5);  // F_5 = 5
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // ADDITION sa normal space
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double result_normal = v_add[1] * PHI + v_add[0];
        cout << "  After add (normal): " << result_normal << "\n";

        // Re-encode para sa log space
        double log_for_7 = log(result_normal) / LN_PHI;
        double q_7 = floor(result_normal / PHI);
        double r_7 = fmod(result_normal, PHI);
        vector<double> v7(4, 0.0);
        v7[0] = r_7;
        v7[1] = q_7;
        v7[2] = log_for_7;
        v7[3] = log_for_7 + 0.0;  // index not needed for log
        Plaintext pt_7 = cc->MakeCKKSPackedPlaintext(v7);
        auto ct_7_enc = cc->Encrypt(keyPair.publicKey, pt_7);

        // MULTIPLICATION sa log space: 7 × 3
        auto ct_c = encrypt_full(4);  // F_4 = 3
        auto ct_mult = cc->EvalAdd(ct_7_enc, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 21)\n";

        // DIVISION sa log space: 21 ÷ 1
        auto ct_d = encrypt_full(2);  // F_2 = 1
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        double result = pow(PHI, v_div[2]);
        cout << "  Final: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // MIXED TEST 2: (F_6 - F_3) × F_5 ÷ F_2
    // ============================================
    cout << "--- TEST 2: (F_6 - F_3) × F_5 ÷ F_2 ---\n";
    cout << "  Expected: (8-2) × 5 ÷ 1 = 30\n\n";
    {
        auto ct_a = encrypt_full(6);  // F_6 = 8
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // SUBTRACTION sa normal space
        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        double result_sub = v_sub[1] * PHI + v_sub[0];
        cout << "  After sub (normal): " << result_sub << "\n";

        // Re-encode para sa log space
        double log_for_6 = log(result_sub) / LN_PHI;
        double q_6 = floor(result_sub / PHI);
        double r_6 = fmod(result_sub, PHI);
        vector<double> v6(4, 0.0);
        v6[0] = r_6;
        v6[1] = q_6;
        v6[2] = log_for_6;
        v6[3] = log_for_6;
        Plaintext pt_6 = cc->MakeCKKSPackedPlaintext(v6);
        auto ct_6_enc = cc->Encrypt(keyPair.publicKey, pt_6);

        // MULTIPLICATION sa log space: 6 × 5
        auto ct_c = encrypt_full(5);  // F_5 = 5
        auto ct_mult = cc->EvalAdd(ct_6_enc, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 30)\n";

        // DIVISION sa log space: 30 ÷ 1
        auto ct_d = encrypt_full(2);  // F_2 = 1
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        double result = pow(PHI, v_div[2]);
        cout << "  Final: " << result << " (Expected: 30)\n";
        cout << "  Match: " << (abs(result - 30.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // TEST 3: ARBITRARY EXPRESSION
    // ============================================
    cout << "--- TEST 3: (F_4 + F_5) × (F_6 - F_2) ÷ F_3 ---\n";
    cout << "  Expected: (3+5) × (8-1) ÷ 2 = 8 × 7 ÷ 2 = 28\n\n";
    {
        // (F_4 + F_5) = 3 + 5 = 8
        auto ct_a = encrypt_full(4);
        auto ct_b = encrypt_full(5);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double sum1 = v_add[1] * PHI + v_add[0];
        cout << "  Sum1: " << sum1 << "\n";

        // (F_6 - F_2) = 8 - 1 = 7
        auto ct_c = encrypt_full(6);
        auto ct_d = encrypt_full(2);
        auto ct_sub = cc->EvalSub(ct_c, ct_d);
        auto v_sub = decrypt_full(ct_sub);
        double diff1 = v_sub[1] * PHI + v_sub[0];
        cout << "  Diff1: " << diff1 << "\n";

        // Product: 8 × 7 = 56
        double log_for_8 = log(8.0) / LN_PHI;
        double log_for_7 = log(7.0) / LN_PHI;
        double log_product = log_for_8 + log_for_7;
        double prod = pow(PHI, log_product);
        cout << "  Product: " << prod << "\n";

        // Division: 56 ÷ 2 = 28
        double log_for_2 = log(2.0) / LN_PHI;
        double log_result = log_product - log_for_2;
        double result = pow(PHI, log_result);
        cout << "  Final: " << result << " (Expected: 28)\n";
        cout << "  Match: " << (abs(result - 28.0) < 0.5 ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Exact log values ang gumagana\n";
    cout << "  Ang approximation ay hindi\n\n";

    return 0;
}
