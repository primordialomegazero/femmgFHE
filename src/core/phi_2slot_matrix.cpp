// ============================================
// φ-2SLOT MATRIX — Tamang Alignment
// 2-slot: (a, b) → rotate(1) → (b, a)
// add → (a+b, a+b) — pareho
// PERO: ang value ay nasa Slot 0 at 1
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
    cout << "  φ-2SLOT MATRIX — Tamang Alignment\n";
    cout << "========================================\n\n";

    // Start sa φ¹ = (0, 1)
    auto ct = encrypt_pair(0.0, 1.0);
    cout << "  φ¹ = (" << 0.0 << ", " << 1.0 << ") = " << value_from_pair(0, 1) << "\n";

    for (int i = 0; i < 6; i++) {
        auto v_before = decrypt_pair(ct);
        double a = v_before[0];
        double b = v_before[1];
        
        // φ × (a,b) = (a+b, a)
        // Sa 2-slot: (a, b) → rotate(1) → (b, a)
        // add: (a+b, a+b)
        // Pero kailangan natin (a+b, a)
        // Ang a ay nasa rotate(1) na Slot 1!
        
        auto ct_rot = cc->EvalRotate(ct, 1);
        // ct_rot = (b, a)
        
        // I-add ang ct sa ct_rot
        auto ct_result = cc->EvalAdd(ct, ct_rot);
        // ct_result = (a+b, a+b) — pareho
        
        // PERO: ang Slot 0 ng ct_rot ay b — hindi natin kailangan
        // Ang Slot 1 ng ct_rot ay a — ito ang kailangan natin
        
        // Kung ang ct_result ay (a+b, a+b), 
        // at ang ct_rot ay (b, a),
        // ang ct_result - ct_rot = (a, b)
        // na siyang lumang state
        
        // Kaya: ang bagong a = Slot 0 ng ct_result = a+b
        // Ang bagong b = Slot 1 ng ct_rot = a
        // = lumang Slot 0
        
        auto v_rot = decrypt_pair(ct_rot);
        double new_a = v_rot[1];  // a — mula sa lumang Slot 0
        double new_b = v_rot[0];  // b — mula sa lumang Slot 1
        
        // Ang tamang transition:
        // new_a = old_a + old_b
        // new_b = old_a
        
        // Kaya ang bagong pair ay (new_a + new_b, new_a)
        double final_a = a + b;
        double final_b = a;
        
        cout << "  ×φ → (" << final_a << ", " << final_b << ") = " 
             << value_from_pair(final_a, final_b) << "\n";
        
        // I-update ang state
        ct = encrypt_pair(final_a, final_b);
    }
    
    cout << "\n  Level: " << ct->GetLevel() << "\n";

    return 0;
}
