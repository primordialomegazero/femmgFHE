// ============================================
// φ-SLOTWISE OPERATIONS
// Hiwalay na operasyon para sa hiwalay na espasyo
// Addition → normal space slots
// Multiplication → log space slot
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

    // ============================================
    // PLAINTEXT MASK PARA SA SLOT-WISE OPERATIONS
    // ============================================

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = n - LOG_SQRT5;
        
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

    auto make_mask = [&](vector<double> mask_vals) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(mask_vals);
        return pt;
    };

    cout << "=== φ-SLOTWISE OPERATIONS ===\n\n";

    // ============================================
    // MIXED TEST 1: (F_5 + F_3) × F_4 ÷ F_2
    // ============================================
    cout << "--- TEST 1: (F_5 + F_3) × F_4 ÷ F_2 ---\n";
    cout << "  Expected: (5+2) × 3 ÷ 1 = 21\n\n";
    {
        auto ct_a = encrypt_full(5);  // F_5 = 5
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // ADDITION: i-preserve ang log at index
        auto mask_normal = make_mask({1.0, 1.0, 0.0, 0.0});
        auto mask_log_idx = make_mask({0.0, 0.0, 1.0, 1.0});
        
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        // Ang normal space ay tama (7), pero kailangan nating i-reconstruct
        // ang bagong state na may tamang log at index
        double result_normal = v_add[1] * PHI + v_add[0];
        cout << "  After add (normal): " << result_normal << "\n";
        cout << "  After add (log): " << v_add[2] << " → φ^ = " << pow(PHI, v_add[2]) << "\n";
        cout << "  After add (index): " << v_add[3] << "\n\n";

        // Para sa multiplication, kailangan nating gamitin ang log space
        // Pero ang log space ay nag-multiply na (5×2=10), hindi nag-add
        // Kaya kailangan nating i-reconstruct muna
        double log_for_7 = log(result_normal) / LN_PHI;
        auto ct_7 = encrypt_full(0);  // placeholder
        
        // I-encode ang 7 bilang bagong state
        double q_7 = floor(result_normal / PHI);
        double r_7 = fmod(result_normal, PHI);
        vector<double> v7(4, 0.0);
        v7[0] = r_7;
        v7[1] = q_7;
        v7[2] = log_for_7;
        v7[3] = log_for_7 + LOG_SQRT5;
        Plaintext pt_7 = cc->MakeCKKSPackedPlaintext(v7);
        auto ct_7_enc = cc->Encrypt(keyPair.publicKey, pt_7);

        // Multiplication: F_7_result × F_4
        auto ct_c = encrypt_full(4);  // F_4 = 3
        auto ct_mult = cc->EvalAdd(ct_7_enc, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 7×3=21)\n\n";

        // Division: prod ÷ F_2 = ÷ 1
        auto ct_d = encrypt_full(2);  // F_2 = 1
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        double result = pow(PHI, v_div[2]);
        cout << "  Final: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 1.0 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // MIXED TEST 2: (F_6 - F_3) × F_5 ÷ F_2
    // ============================================
    cout << "--- TEST 2: (F_6 - F_3) × F_5 ÷ F_2 ---\n";
    cout << "  Expected: (8-2) × 5 ÷ 1 = 30\n\n";
    {
        auto ct_a = encrypt_full(6);  // F_6 = 8
        auto ct_b = encrypt_full(3);  // F_3 = 2

        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        double result_sub = v_sub[1] * PHI + v_sub[0];
        cout << "  After sub (normal): " << result_sub << "\n";

        double log_for_6 = log(result_sub) / LN_PHI;
        double q_6 = floor(result_sub / PHI);
        double r_6 = fmod(result_sub, PHI);
        vector<double> v6(4, 0.0);
        v6[0] = r_6;
        v6[1] = q_6;
        v6[2] = log_for_6;
        v6[3] = log_for_6 + LOG_SQRT5;
        Plaintext pt_6 = cc->MakeCKKSPackedPlaintext(v6);
        auto ct_6_enc = cc->Encrypt(keyPair.publicKey, pt_6);

        auto ct_c = encrypt_full(5);  // F_5 = 5
        auto ct_mult = cc->EvalAdd(ct_6_enc, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 6×5=30)\n";

        auto ct_d = encrypt_full(2);  // F_2 = 1
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        double result = pow(PHI, v_div[2]);
        cout << "  Final: " << result << " (Expected: 30)\n";
        cout << "  Match: " << (abs(result - 30.0) < 1.0 ? "✅" : "❌") << "\n\n";
    }

    cout << "=== KEY ===\n";
    cout << "  Ang mixed operations ay nangangailangan ng\n";
    cout << "  re-encoding pagkatapos ng normal-space operation\n";
    cout << "  para i-sync ang log space\n\n";

    return 0;
}
