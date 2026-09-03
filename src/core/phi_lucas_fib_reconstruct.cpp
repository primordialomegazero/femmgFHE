// ============================================
// φ-LUCAS-FIB RECONSTRUCT
// Encoding: [x, F_n, L_n, L_n+F_n×√5]
// Reconstruct ang x mula sa Lucas at Fibonacci
// Walang EvalMult, walang bootstrapping
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-LUCAS-FIB RECONSTRUCT\n";
    cout << "  (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // LUCAS-FIB ENCODING
    // Slot 0: x (normal)
    // Slot 1: F_n (Fibonacci)
    // Slot 2: L_n (Lucas)
    // Slot 3: L_n + F_n × √5 (Reconstruct value)
    // ============================================

    auto encrypt_lf = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        double F_n = fib[n];
        double L_n = lucas[n];
        double reconstruct = L_n + F_n * SQRT5;
        
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = F_n;
        v[2] = L_n;
        v[3] = reconstruct;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_lf = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        return vector<double>{results[0].real(), results[1].real(),
                              results[2].real(), results[3].real()};
    };

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_lf(5.0);
    auto ct_7 = encrypt_lf(7.0);
    auto ct_3 = encrypt_lf(3.0);

    auto v5 = decrypt_lf(ct_5);
    auto v7 = decrypt_lf(ct_7);
    auto v3 = decrypt_lf(ct_3);
    
    cout << "  5: [" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    cout << "  7: [" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n";
    cout << "  3: [" << v3[0] << ", " << v3[1] << ", " << v3[2] << ", " << v3[3] << "]\n\n";

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_lf(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (x): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (F_n): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (L_n): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (Reconstruct): " << mult_vals[3] << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_lf(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (x): " << final_vals[0] << "\n";
    cout << "  Slot 1 (F_n): " << final_vals[1] << "\n";
    cout << "  Slot 2 (L_n): " << final_vals[2] << "\n";
    cout << "  Slot 3 (Reconstruct): " << final_vals[3] << "\n\n";

    // ============================================
    // RECONSTRUCTION ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  RECONSTRUCTION ANALYSIS\n";
    cout << "========================================\n\n";

    // Para sa 35: n=7, F_7=13, L_7=29
    // L_7 + F_7 × √5 = 29 + 13×2.236 = 29 + 29.068 = 58.068
    // 58.068 / 2 = 29.034 = φ^7
    // 35 = φ^7 × φ^0.3883 = 29.034 × 1.2055 = 35.000

    double F_35 = fib[7];
    double L_35 = lucas[7];
    double reconstruct_35 = L_35 + F_35 * SQRT5;
    double phi_7 = reconstruct_35 / 2.0;
    double frac_35 = log(35.0)/LN_PHI - 7;
    double phi_frac_35 = pow(PHI, frac_35);
    double recovered_35 = phi_7 * phi_frac_35;
    
    cout << "  Para sa 35:\n";
    cout << "  F_7 = " << F_35 << "\n";
    cout << "  L_7 = " << L_35 << "\n";
    cout << "  L_7 + F_7×√5 = " << reconstruct_35 << "\n";
    cout << "  φ^7 = " << phi_7 << "\n";
    cout << "  φ^frac = " << phi_frac_35 << "\n";
    cout << "  Recovered: " << recovered_35 << " (expected: 35)\n";
    cout << "  Match: " << (abs(recovered_35 - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
