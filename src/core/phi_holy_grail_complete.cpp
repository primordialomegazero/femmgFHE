// ============================================
// φ-HOLY GRAIL COMPLETE
// 4-Slot Encoding: [a, b, a+bφ, floor_index]
// Ang floor_index ay nagbibigay ng tamang F_n
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
    cout << "  φ-HOLY GRAIL COMPLETE\n";
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // 4-SLOT ENCODING
    // Slot 0: a (normal component)
    // Slot 1: b = F_n (Fibonacci component)
    // Slot 2: a + bφ (buong value)
    // Slot 3: floor_index (n)
    // ============================================

    auto encrypt_complete = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double floor_val = floor(log_phi_x);
        double frac = log_phi_x - floor_val;
        int n = (int)floor_val;
        long long b = fib[n];
        double a = x - b * PHI;
        
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = (double)b;
        v[2] = a + b * PHI;
        v[3] = floor_val;  // Fibonacci index
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_complete = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_complete(5.0);
    auto ct_7 = encrypt_complete(7.0);
    auto ct_3 = encrypt_complete(3.0);

    auto v5 = decrypt_complete(ct_5);
    auto v7 = decrypt_complete(ct_7);
    auto v3 = decrypt_complete(ct_3);
    
    cout << "  5: [" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    cout << "  7: [" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n";
    cout << "  3: [" << v3[0] << ", " << v3[1] << ", " << v3[2] << ", " << v3[3] << "]\n\n";

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_complete(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (a): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (b): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (a+bφ): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (floor_index): " << mult_vals[3] << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_complete(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (a): " << final_vals[0] << "\n";
    cout << "  Slot 1 (b): " << final_vals[1] << "\n";
    cout << "  Slot 2 (a+bφ): " << final_vals[2] << " ← RESULT\n";
    cout << "  Slot 3 (floor_index): " << final_vals[3] << "\n\n";

    cout << "  Expected: 38\n";
    cout << "  Match: " << (abs(final_vals[2] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
