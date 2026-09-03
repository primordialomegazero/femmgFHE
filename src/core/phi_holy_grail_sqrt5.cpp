// ============================================
// φ-HOLY GRAIL SQRT5
// Slot 1: φ^n/√5 (approximation ng F_n)
// Ang φ^n/√5 ay may property na additive sa log space
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
    cout << "  φ-HOLY GRAIL SQRT5\n";
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

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // ENCODING: [a, φ^n/√5, a+bφ, n]
    // ============================================

    auto encrypt_sqrt5 = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double floor_val = floor(log_phi_x);
        int n = (int)floor_val;
        double phi_n = pow(PHI, n);
        double b_approx = phi_n / SQRT5;
        long long b_exact = round(b_approx);
        double a = x - b_exact * PHI;
        
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b_approx;       // φ^n/√5
        v[2] = a + b_exact * PHI;  // buong value
        v[3] = floor_val;      // n
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_sqrt5 = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_sqrt5(5.0);
    auto ct_7 = encrypt_sqrt5(7.0);
    auto ct_3 = encrypt_sqrt5(3.0);

    auto v5 = decrypt_sqrt5(ct_5);
    auto v7 = decrypt_sqrt5(ct_7);
    auto v3 = decrypt_sqrt5(ct_3);
    
    cout << "  5: [" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    cout << "  7: [" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n";
    cout << "  3: [" << v3[0] << ", " << v3[1] << ", " << v3[2] << ", " << v3[3] << "]\n\n";

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_sqrt5(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (a): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (φ^n/√5): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (a+bφ): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (n): " << mult_vals[3] << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_sqrt5(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (a): " << final_vals[0] << "\n";
    cout << "  Slot 1 (φ^n/√5): " << final_vals[1] << "\n";
    cout << "  Slot 2 (a+bφ): " << final_vals[2] << " ← RESULT\n";
    cout << "  Slot 3 (n): " << final_vals[3] << "\n\n";

    cout << "  Expected: 38\n";
    cout << "  Match: " << (abs(final_vals[2] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
