// ============================================
// φ-CORRECT TRANSITION
// (a, b) → (b, a+b)
// φ^n = (F_{n-1}, F_n)
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;

    auto value_from_pair = [&](double a, double b) {
        return a + b * PHI;
    };

    auto encrypt_pair = [&](double a, double b) {
        vector<double> v(2, 0.0);
        v[0] = a;
        v[1] = b;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-CORRECT TRANSITION\n";
    cout << "========================================\n\n";
    cout << "  (a, b) → (b, a+b)\n";
    cout << "  φ^n = (F_{n-1}, F_n)\n\n";

    // φ¹ = (F₀, F₁) = (0, 1)
    auto ct = encrypt_pair(0.0, 1.0);
    cout << "  φ¹ = (0, 1) = " << value_from_pair(0, 1) << "\n";

    for (int i = 0; i < 8; i++) {
        auto v = decrypt_pair(ct);
        double a = v[0];  // F_{n-1}
        double b = v[1];  // F_n
        
        // Transition: (a, b) → (b, a+b)
        // Bagong Slot 0 = b — ito ay mula sa rotate(1) na Slot 0
        // Bagong Slot 1 = a+b — kailangan ng addition
        
        // Sa 2-slot:
        // rotate(1): (b, a) — swap
        // Ang bagong pair ay (b, a+b)
        // = (rotate(1) Slot 0, original Slot 0 + original Slot 1)
        
        // Kaya: ct_new = (rotate(ct, 1) Slot 0, ct Slot 0 + ct Slot 1)
        // = (b, a+b)
        
        auto ct_rot = cc->EvalRotate(ct, 1);
        auto ct_add = cc->EvalAdd(ct, ct_rot);
        
        // ct_add = (a+b, a+b) — pareho
        // ct_rot = (b, a) — swapped
        
        // Ang bagong pair: (ct_rot[0], ct_add[0]) = (b, a+b)
        auto v_rot = decrypt_pair(ct_rot);
        auto v_add = decrypt_pair(ct_add);
        
        double new_a = v_rot[0];  // b
        double new_b = v_add[0];  // a+b
        
        cout << "  ×φ → (" << new_a << ", " << new_b << ") = " 
             << value_from_pair(new_a, new_b) << "\n";
        cout << "  Expected φ^" << i+2 << " = " << pow(PHI, i+2) << "\n";
        
        // I-update
        ct = encrypt_pair(new_a, new_b);
    }
    
    cout << "\n  Level: " << ct->GetLevel() << "\n";

    return 0;
}
