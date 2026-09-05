// ============================================
// φ-SELF-REF MODE — 20 iterations
// Self-referential: ang mode ay naka-encode sa state
// Even n → addition mode
// Odd n → multiplication mode
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

    auto encrypt_state = [&](double n, double value) {
        vector<double> v(2, 0.0);
        v[0] = n;       // exponent — nagbibigay ng mode
        v[1] = value;   // actual value
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
    cout << "  φ-SELF-REF MODE — 20 iterations\n";
    cout << "========================================\n\n";
    cout << "  Even n → addition mode\n";
    cout << "  Odd n → multiplication mode\n";
    cout << "  Self-referential: φ ang nagbibigay ng mode\n\n";

    // Start sa value=5, n=0 (even → addition mode)
    auto ct_state = encrypt_state(0.0, 5.0);

    vector<double> delta_n(2, 0.0);
    delta_n[0] = 1.0;  // +1 sa exponent — mode switch
    Plaintext pt_n = cc->MakeCKKSPackedPlaintext(delta_n);

    cout << "  Initial: value=5, mode=addition\n\n";

    for (int i = 0; i < 10; i++) {
        // Move sa cycle
        ct_state = cc->EvalAdd(ct_state, pt_n);
        
        auto v = decrypt_state(ct_state);
        int parity = ((int)round(v[0])) % 2;
        string mode = (parity == 0) ? "addition" : "multiplication";
        
        cout << "  Step " << setw(2) << i << ": n=" << setw(3) << v[0]
             << ", value=" << setw(10) << v[1]
             << ", mode=" << mode << "\n";
    }

    cout << "\n  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
