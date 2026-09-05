// ============================================
// φ-PARITY TOGGLE — 20 iterations
// φ^parity: 1 (even), φ (odd)
// Automatic toggle sa φ-structure
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

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double log_val, double parity_val) {
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = parity_val;  // 1 (even) o φ (odd)
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
    cout << "  φ-PARITY TOGGLE — 20 iterations\n";
    cout << "========================================\n\n";
    cout << "  Slot 0: log\n";
    cout << "  Slot 1: φ^parity (1=even, φ=odd)\n";
    cout << "  Automatic toggle sa φ-structure\n\n";

    // Start: log=1, parity=odd (φ)
    auto ct_state = encrypt_state(1.0, PHI);

    // Multiply delta: +1 sa log, ×φ sa parity
    vector<double> delta_mul(2, 0.0);
    delta_mul[0] = 1.0;
    delta_mul[1] = 0.0;  // parity ay magbabago sa pamamagitan ng multiply
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(delta_mul);

    cout << "  Initial: log=1, parity=φ (odd)\n\n";

    for (int i = 0; i < 10; i++) {
        ct_state = cc->EvalAdd(ct_state, pt_mul);
        
        auto v = decrypt_state(ct_state);
        
        // Ang parity: kung ang value ay φ, odd; kung 1 o φ², even
        string parity_str;
        double parity_val = v[1];
        double mod_phi = fmod(parity_val, PHI);
        
        if (abs(mod_phi - 1.0) < 0.01) parity_str = "even";
        else if (abs(mod_phi - (PHI - 1.0)) < 0.01) parity_str = "odd";
        else parity_str = "?";
        
        cout << "  Step " << setw(2) << i << ": log=" << setw(4) << v[0]
             << ", parity=" << setw(8) << parity_val 
             << ", mod_φ=" << setw(8) << mod_phi
             << ", state=" << parity_str << "\n";
    }

    cout << "\n  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
