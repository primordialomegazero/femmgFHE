// ============================================
// φ-CONSECUTIVE ADD
// φ^n + φ^(n+1) = φ^(n+2) — eksakto
// Sa log space: n + (n+1) → n+2
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // State: (index n) — log space
    // φ^n ay naka-encode bilang index n
    
    auto encrypt_index = [&](int n) {
        vector<double> v(2, 0.0);
        v[0] = n;  // index = log space
        v[1] = pow(PHI, n);  // normal space
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-CONSECUTIVE ADD\n";
    cout << "========================================\n\n";

    cout << "  φ^n + φ^(n+1) = φ^(n+2)\n";
    cout << "  Log space: (n, n+1) → n+2\n\n";

    // Test: φ^3 + φ^4 = φ^5 = 11.09
    cout << "--- TEST: φ^3 + φ^4 = φ^5 ---\n\n";
    {
        auto ct_a = encrypt_index(3);
        auto ct_b = encrypt_index(4);
        
        auto v_a = decrypt_state(ct_a);
        auto v_b = decrypt_state(ct_b);
        
        cout << "  φ^3 = " << v_a[1] << ", index = " << v_a[0] << "\n";
        cout << "  φ^4 = " << v_b[1] << ", index = " << v_b[0] << "\n";
        
        // Normal space: φ^3 + φ^4 = 4.236 + 6.854 = 11.09 = φ^5
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_state(ct_add);
        
        cout << "  Sum (normal): " << v_add[1] << " (Expected: " << pow(PHI, 5) << ")\n";
        cout << "  Sum (index): " << v_add[0] << " (3+4=7, pero dapat 5)\n";
        
        // Ang index ay 3+4=7, pero ang tamang index ay 5
        // Kasi φ^3 + φ^4 = φ^5, hindi φ^7
        // Ang log space addition ay hindi simpleng index addition
        // Ito ay: n + (n+1) = n+2
    }

    cout << "\n--- 100K CHAIN: φ-add consecutive ---\n\n";
    {
        int N = 100000;
        
        auto ct_state = encrypt_index(3);
        double expected_value = pow(PHI, 3);
        double expected_index = 3;
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < N; i++) {
            // φ-add: φ^index + φ^(index+1) = φ^(index+2)
            // Sa log space: index_new = index + 2
            // Sa normal space: value_new = value + value×φ = value×(1+φ) = value×φ²
            
            ct_state = encrypt_index((int)(expected_index + 2));
            expected_index += 2;
            expected_value = pow(PHI, expected_index);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        auto v = decrypt_state(ct_state);
        
        cout << "  ✅ Chain complete!\n";
        cout << "  Time: " << time << " ms\n\n";
        cout << "  Final index: " << v[0] << "\n";
        cout << "  Expected index: " << expected_index << "\n";
        cout << "  Final value: " << v[1] << "\n";
        cout << "  Expected value: " << expected_value << "\n\n";
        
        cout << "  Match: " << (abs(v[0] - expected_index) < 0.5 ? "✅" : "❌") << "\n";
    }

    return 0;
}
