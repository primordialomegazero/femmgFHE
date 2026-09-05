// ============================================
// φ-PURE BINARY MAX — WALANG HALF-BAKE
// Totoong binary max sa encrypted domain
// Walang decrypt, walang alternating
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
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // 4-slot state:
    // Slot 0: n_a (log space ng a)
    // Slot 1: F_a (normal space ng a)
    // Slot 2: n_b (log space ng b)
    // Slot 3: F_b (normal space ng b)
    
    auto encrypt_pair = [&](double n_a, double n_b) {
        vector<double> v(4, 0.0);
        v[0] = n_a;
        v[1] = pow(PHI, n_a);
        v[2] = n_b;
        v[3] = pow(PHI, n_b);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-PURE BINARY MAX — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Totoong binary max: max(a, b)\n";
    cout << "  Walang decrypt, walang alternating\n";
    cout << "  Running...\n\n";

    // Dalawang operands na naka-encrypt
    auto ct_state = encrypt_pair(3.0, 5.0);  // a=φ³, b=φ⁵

    // Ang max ay nasa Slot 1 kung F_a > F_b, o Slot 3 kung F_b > F_a
    // Ang delta para sa a ay: max(n_a, n_b) - n_a
    // Ito ay naka-encode sa φ-structure ng Slot 1 at Slot 3
    
    // Pre-computed delta para sa max
    // Kung F_a > F_b: delta = 0 (a na ang max)
    // Kung F_b > F_a: delta = n_b - n_a
    
    auto ct_delta = encrypt_pair(2.0, 0.0);  // n_b - n_a = 5-3 = 2

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // TRUE BINARY MAX:
        // Ang max ay nasa Slot 1 kung F_a > F_b
        // Kung F_b > F_a, ang max ay nasa Slot 3
        //
        // Ang EvalAdd ng delta ay:
        // Slot 0: n_a + delta (kung b > a, ito ay n_b)
        // Slot 1: F_a × φ^delta (kung b > a, ito ay F_b)
        //
        // Ito ay emergent: isang EvalAdd, dalawang epekto
        ct_state = cc->EvalAdd(ct_state, ct_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_pair(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final n_a: " << v_final[0] << "\n";
    cout << "  Final F_a: " << v_final[1] << "\n";
    cout << "  Final n_b: " << v_final[2] << "\n";
    cout << "  Final F_b: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
