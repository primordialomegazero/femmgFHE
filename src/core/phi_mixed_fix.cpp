// ============================================
// φ-MIXED FIX
// Index slot bilang tulay sa mixed operations
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

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = n - LOG_SQRT5;  // Approximation
        
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

    cout << "=== φ-MIXED FIX ===\n\n";
    cout << "  Using log_φ(F_n) ≈ n - log_φ(√5)\n";
    cout << "  log_φ(√5) = " << LOG_SQRT5 << "\n\n";

    // ============================================
    // MIXED TEST 1: (F_5 + F_3) × F_4 ÷ F_2
    // ============================================
    cout << "--- MIXED TEST 1: (F_5 + F_3) × F_4 ÷ F_2 ---\n";
    cout << "  Expected: (5 + 2) × 3 ÷ 1 = 21\n\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        // Ang index ay 5+3=8, pero ang value ay F_5+F_3=7
        // Kailangan nating hanapin ang tamang index
        // 7 ay hindi Fibonacci number, kaya approximation
        double approx_index = log(7.0) / LN_PHI + LOG_SQRT5;
        
        cout << "  After add: index=" << v_add[3] << ", log_phi=" << v_add[2] << "\n";
        cout << "  Approx index for value 7: " << approx_index << "\n\n";
        
        // Kailangan nating i-encode ang result bilang bagong state
        // na may tamang index
        auto ct_c = encrypt_full(4);
        auto ct_mult = cc->EvalAdd(ct_add, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        
        cout << "  After mult: log_phi=" << v_mult[2] << "\n";
        cout << "  Expected log_phi for 21: " << log(21.0)/LN_PHI << "\n\n";
        
        auto ct_d = encrypt_full(2);
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        
        double result = pow(PHI, v_div[2]);
        cout << "  After div: log_phi=" << v_div[2] << "\n";
        cout << "  Result: " << result << " (Expected: 21)\n\n";
    }

    // ============================================
    // MIXED TEST 2: (F_6 - F_3) × F_5 ÷ F_2
    // ============================================
    cout << "--- MIXED TEST 2: (F_6 - F_3) × F_5 ÷ F_2 ---\n";
    cout << "  Expected: (8 - 2) × 5 ÷ 1 = 30\n\n";
    {
        auto ct_a = encrypt_full(6);
        auto ct_b = encrypt_full(3);
        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        
        auto ct_c = encrypt_full(5);
        auto ct_mult = cc->EvalAdd(ct_sub, ct_c);
        auto v_mult = decrypt_full(ct_mult);
        
        auto ct_d = encrypt_full(2);
        auto ct_div = cc->EvalSub(ct_mult, ct_d);
        auto v_div = decrypt_full(ct_div);
        
        double result = pow(PHI, v_div[2]);
        cout << "  Result: " << result << " (Expected: 30)\n\n";
    }

    cout << "=== KEY INSIGHT ===\n";
    cout << "  Ang log_φ approximation na may index ay\n";
    cout << "  kailangang i-update pagkatapos ng bawat operasyon\n";
    cout << "  kasi ang index slot ay hindi nagre-reconstruct\n";
    cout << "  ng bagong value nang mag-isa\n\n";

    return 0;
}
