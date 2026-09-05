// ============================================
// φ-OPERATOR — 100 iterations
// Ang φ bilang operator, hindi value
// State = posisyon sa cycle
// EvalAdd = move sa cycle
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

    // State bilang operator:
    // Slot 0: exponent n (posisyon sa cycle)
    // Slot 1: φ^n (value na sumusunod)
    
    auto encrypt_operator = [&](double n) {
        vector<double> v(2, 0.0);
        v[0] = n;
        v[1] = pow(PHI, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_operator = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-OPERATOR — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  State = posisyon sa cycle\n";
    cout << "  φ ang operator ng transformation\n";
    cout << "  EvalAdd = move sa cycle\n\n";

    // Start sa n=0 (φ^0 = 1, even)
    auto ct_state = encrypt_operator(0.0);

    vector<double> delta(2, 0.0);
    delta[0] = 1.0;    // +1 sa exponent
    delta[1] = 0.0;
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);

    cout << "  Cycle: n=0 (even) → n=1 (odd) → n=2 (even) → ...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        ct_state = cc->EvalAdd(ct_state, pt_delta);
        
        auto v = decrypt_operator(ct_state);
        
        // Ang parity ay mula sa exponent: even → 0, odd → 1
        int parity = ((int)round(v[0])) % 2;
        string state_str = (parity == 0) ? "even" : "odd";
        
        // Ang value ay automatic: φ^n
        double value = pow(PHI, v[0]);
        
        cout << "  Step " << setw(2) << i << ": n=" << setw(4) << v[0]
             << ", φ^n=" << setw(12) << value
             << ", state=" << state_str
             << ", mod_φ=" << setw(10) << fmod(value, PHI) << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
