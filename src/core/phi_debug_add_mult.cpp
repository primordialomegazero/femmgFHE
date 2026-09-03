// ============================================
// φ-DEBUG ADD+MULT
// I-debug ang addition + multiplication
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

    cout << "=== φ-DEBUG ADD+MULT ===\n\n";

    // ============================================
    // STEP 1: ADDITION
    // ============================================
    cout << "--- STEP 1: ADDITION (F_5 + F_3) ---\n\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        
        cout << "  F_5 = 5: ";
        auto v5 = decrypt_full(ct_a);
        cout << "r=" << v5[0] << " q=" << v5[1] << " log=" << v5[2] << " idx=" << v5[3] << "\n";
        
        cout << "  F_3 = 2: ";
        auto v3 = decrypt_full(ct_b);
        cout << "r=" << v3[0] << " q=" << v3[1] << " log=" << v3[2] << " idx=" << v3[3] << "\n";
        
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        cout << "  Sum: ";
        cout << "r=" << v_add[0] << " q=" << v_add[1] << " log=" << v_add[2] << " idx=" << v_add[3] << "\n";
        
        // Ang normal space sum ay q×φ+r = 4×φ+0.528 = 7
        double F_sum = v_add[1] * PHI + v_add[0];
        cout << "  F_sum = " << F_sum << "\n\n";
        
        // Bridge
        double log_F_bridge = log_q_table[(int)v_add[1]] + 1.0 + log(1.0 + v_add[0] / (v_add[1] * PHI)) / LN_PHI;
        cout << "  Bridge log_F = " << log_F_bridge << "\n";
        cout << "  Exact log_φ(7) = " << log(7.0) / LN_PHI << "\n\n";
        
        // ============================================
        // STEP 2: MULTIPLICATION (7 × 3)
        // ============================================
        cout << "--- STEP 2: MULTIPLICATION (7 × 3) ---\n\n";
        
        // I-encode ang 7 gamit ang bridge log
        vector<double> v_7(4, 0.0);
        v_7[2] = log_F_bridge;
        Plaintext pt_7 = cc->MakeCKKSPackedPlaintext(v_7);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, pt_7);
        
        cout << "  7 (log only): ";
        auto v7 = decrypt_full(ct_7);
        cout << "r=" << v7[0] << " q=" << v7[1] << " log=" << v7[2] << " idx=" << v7[3] << "\n";
        
        // I-encode ang 3 gamit ang log
        vector<double> v_3(4, 0.0);
        v_3[2] = log(3.0) / LN_PHI;
        Plaintext pt_3 = cc->MakeCKKSPackedPlaintext(v_3);
        auto ct_3 = cc->Encrypt(keyPair.publicKey, pt_3);
        
        cout << "  3 (log only): ";
        auto v3b = decrypt_full(ct_3);
        cout << "r=" << v3b[0] << " q=" << v3b[1] << " log=" << v3b[2] << " idx=" << v3b[3] << "\n";
        
        // EvalAdd para sa multiplication
        auto ct_mult = cc->EvalAdd(ct_7, ct_3);
        auto v_mult = decrypt_full(ct_mult);
        
        cout << "  Product (log): ";
        cout << "r=" << v_mult[0] << " q=" << v_mult[1] << " log=" << v_mult[2] << " idx=" << v_mult[3] << "\n";
        
        double result = pow(PHI, v_mult[2]);
        cout << "  Result: " << result << " (Expected: 21)\n";
        
        // Tignan kung may mali sa log values
        cout << "\n  Expected log_sum = " << log_F_bridge + log(3.0)/LN_PHI << "\n";
        cout << "  Actual log_sum = " << v_mult[2] << "\n";
        cout << "  Difference = " << (v_mult[2] - (log_F_bridge + log(3.0)/LN_PHI)) << "\n";
    }

    return 0;
}
