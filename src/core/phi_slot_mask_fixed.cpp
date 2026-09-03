// ============================================
// φ-SLOT MASK FIXED
// Multiplication: Slot 2 lang ang binabago
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
        v[2] = log(F) / LN_PHI;  // Exact log for Fibonacci
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

    // Para sa multiplication, gumawa tayo ng ciphertext na Slot 2 lang ang may laman
    auto encrypt_log_only = [&](int n) {
        double F = (double)fib[n];
        vector<double> v(4, 0.0);
        v[0] = 0;
        v[1] = 0;
        v[2] = log(F) / LN_PHI;  // Log value
        v[3] = 0;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    cout << "=== φ-SLOT MASK FIXED ===\n\n";

    // ============================================
    // TEST 1: (F_5 + F_3) × F_4 = 21
    // ============================================
    cout << "--- TEST 1: (F_5 + F_3) × F_4 = 21 ---\n\n";
    {
        auto ct_a = encrypt_full(5);  // F_5 = 5
        auto ct_b = encrypt_full(3);  // F_3 = 2

        // Addition sa normal space
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double F_add = v_add[1] * PHI + v_add[0];  // = 7
        
        cout << "  After add (F): " << F_add << "\n";
        
        // Bridge para sa arbitrary F
        double q_add = floor(F_add / PHI);
        double r_add = fmod(F_add, PHI);
        double corr_add = log(1.0 + r_add / (q_add * PHI)) / LN_PHI;
        double log_F_bridge = log_q_table[(int)q_add] + 1.0 + corr_add;
        
        cout << "  log_φ(F) via bridge: " << log_F_bridge << "\n";
        cout << "  log_φ(F) exact: " << log(F_add) / LN_PHI << "\n\n";

        // I-encode ang transitioned state na may Slot 2 lang
        vector<double> v_trans(4, 0.0);
        v_trans[0] = 0;
        v_trans[1] = 0;
        v_trans[2] = log_F_bridge;
        v_trans[3] = 0;
        Plaintext pt_trans = cc->MakeCKKSPackedPlaintext(v_trans);
        auto ct_trans = cc->Encrypt(keyPair.publicKey, pt_trans);

        // Multiplication: 7 × 3
        auto ct_c = encrypt_log_only(4);  // F_4 = 3, Slot 2 lang
        auto ct_mult = cc->EvalAdd(ct_trans, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 21)\n";
        cout << "  Match: " << (abs(prod - 21.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // TEST 2: (F_6 - F_3) × F_5 = 30
    // ============================================
    cout << "--- TEST 2: (F_6 - F_3) × F_5 = 30 ---\n\n";
    {
        auto ct_a = encrypt_full(6);  // F_6 = 8
        auto ct_b = encrypt_full(3);  // F_3 = 2

        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        double F_sub = v_sub[1] * PHI + v_sub[0];  // = 6
        
        cout << "  After sub (F): " << F_sub << "\n";
        
        double q_sub = floor(F_sub / PHI);
        double r_sub = fmod(F_sub, PHI);
        double corr_sub = log(1.0 + r_sub / (q_sub * PHI)) / LN_PHI;
        double log_F_bridge = log_q_table[(int)q_sub] + 1.0 + corr_sub;
        
        cout << "  log_φ(F) via bridge: " << log_F_bridge << "\n";
        cout << "  log_φ(F) exact: " << log(F_sub) / LN_PHI << "\n\n";

        vector<double> v_trans(4, 0.0);
        v_trans[0] = 0;
        v_trans[1] = 0;
        v_trans[2] = log_F_bridge;
        v_trans[3] = 0;
        Plaintext pt_trans = cc->MakeCKKSPackedPlaintext(v_trans);
        auto ct_trans = cc->Encrypt(keyPair.publicKey, pt_trans);

        auto ct_c = encrypt_log_only(5);  // F_5 = 5, Slot 2 lang
        auto ct_mult = cc->EvalAdd(ct_trans, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        double prod = pow(PHI, v_mult[2]);
        cout << "  After mult: " << prod << " (Expected: 30)\n";
        cout << "  Match: " << (abs(prod - 30.0) < 0.5 ? "✅" : "❌") << "\n\n";
    }

    cout << "=== KEY ===\n";
    cout << "  Ang Slot 2 lang ang kailangan para sa multiplication\n";
    cout << "  Ang ibang slots ay dapat naka-mask\n\n";

    return 0;
}
