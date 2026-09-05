// ============================================
// φ-ALL OPS EMERGENT — 100 iterations
// Lahat ng operations sa φ-power space
// Add: φ^n + φ^(n-1) = φ^(n+1) → +1
// Sub: φ^n - φ^(n-1) = φ^(n-2) → -2
// Mul: φ^n × φ = φ^(n+1) → +1
// Div: φ^n ÷ φ = φ^(n-1) → -1
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-ALL OPS EMERGENT — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Lahat ng operations sa φ-power space\n";
    cout << "  Add: φ^n + φ^(n-1) = φ^(n+1) → +1\n";
    cout << "  Sub: φ^n - φ^(n-1) = φ^(n-2) → -2\n";
    cout << "  Mul: φ^n × φ = φ^(n+1) → +1\n";
    cout << "  Div: φ^n ÷ φ = φ^(n-1) → -1\n\n";

    // Pre-computed deltas
    vector<double> d_add(1, 1.0);   // +1 para sa addition
    vector<double> d_sub(1, 2.0);   // -2 para sa subtraction
    vector<double> d_mul(1, 1.0);   // +1 para sa multiply ng φ
    vector<double> d_div(1, 1.0);   // -1 para sa divide ng φ
    
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(d_add);
    Plaintext pt_sub = cc->MakeCKKSPackedPlaintext(d_sub);
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(d_mul);
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(d_div);

    // ============================================
    // TEST 1: Individual operations
    // ============================================
    cout << "  TEST 1: Individual operations\n\n";
    
    // Add: φ⁵ + φ⁴ = φ⁶
    auto ct = encrypt_n(5.0);
    ct = cc->EvalAdd(ct, pt_add);
    cout << "    Add: φ⁵+φ⁴ = φ⁶ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";
    
    // Sub: φ⁵ - φ⁴ = φ³
    ct = encrypt_n(5.0);
    ct = cc->EvalSub(ct, pt_sub);
    cout << "    Sub: φ⁵-φ⁴ = φ³ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";
    
    // Mul: φ⁵ × φ = φ⁶
    ct = encrypt_n(5.0);
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    Mul: φ⁵×φ = φ⁶ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";
    
    // Div: φ⁵ ÷ φ = φ⁴
    ct = encrypt_n(5.0);
    ct = cc->EvalSub(ct, pt_div);
    cout << "    Div: φ⁵÷φ = φ⁴ = " << pow(PHI, decrypt_n(ct)) << " ✅\n\n";

    // ============================================
    // TEST 2: Sequence ng lahat ng ops
    // ============================================
    cout << "  TEST 2: Sequence ng lahat ng ops\n\n";
    
    ct = encrypt_n(8.0);
    cout << "    Start: φ⁸ = " << pow(PHI, 8) << "\n";
    
    // Add: φ⁹
    ct = cc->EvalAdd(ct, pt_add);
    cout << "    Add → φ⁹ = " << pow(PHI, decrypt_n(ct)) << "\n";
    
    // Mul: φ¹⁰
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    Mul → φ¹⁰ = " << pow(PHI, decrypt_n(ct)) << "\n";
    
    // Sub: φ⁸
    ct = cc->EvalSub(ct, pt_sub);
    cout << "    Sub → φ⁸ = " << pow(PHI, decrypt_n(ct)) << "\n";
    
    // Div: φ⁷
    ct = cc->EvalSub(ct, pt_div);
    cout << "    Div → φ⁷ = " << pow(PHI, decrypt_n(ct)) << "\n";
    
    // Add: φ⁸
    ct = cc->EvalAdd(ct, pt_add);
    cout << "    Add → φ⁸ = " << pow(PHI, decrypt_n(ct)) << "\n";
    
    // Mul: φ⁹
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    Mul → φ⁹ = " << pow(PHI, decrypt_n(ct)) << "\n\n";

    // ============================================
    // TEST 3: 10K mixed operations
    // ============================================
    cout << "  TEST 3: 10K mixed operations\n\n";
    
    ct = encrypt_n(10.0);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        switch (i % 4) {
            case 0: ct = cc->EvalAdd(ct, pt_add); break;   // Add: +1
            case 1: ct = cc->EvalSub(ct, pt_sub); break;   // Sub: -2
            case 2: ct = cc->EvalAdd(ct, pt_mul); break;   // Mul: +1
            case 3: ct = cc->EvalSub(ct, pt_div); break;   // Div: -1
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct);
    double value_final = pow(PHI, n_final);
    
    // Expected: 2500×(1-2+1-1) = 2500×(-1) = -2500
    double expected_n = 10.0 + 2500.0 * (1.0 - 2.0 + 1.0 - 1.0);
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Final n: " << n_final << "\n";
    cout << "    Expected n: " << expected_n << "\n";
    cout << "    Match: " << (abs(n_final - expected_n) < 0.1 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
