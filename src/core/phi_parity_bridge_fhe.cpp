// ============================================
// φ-PARITY BRIDGE FHE — 100 iterations
// Slot 0: log value
// Slot 1: φ^parity (1 for even, φ for odd)
// EvalAdd = multiply, EvalSub = divide
// Ang parity ay emergent sa φ-power
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

    auto encrypt_state = [&](double log_val, double parity_phi) {
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = parity_phi;   // 1 for even, φ for odd
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
    cout << "  φ-PARITY BRIDGE FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Slot 0: log value\n";
    cout << "  Slot 1: φ^parity (1=even, φ=odd)\n\n";

    // Start: log=1 (φ^1), parity=φ (odd)
    auto ct_state = encrypt_state(1.0, PHI);

    vector<double> delta_add(2, 0.0);
    delta_add[0] = 1.0;    // multiply ng φ
    delta_add[1] = 0.0;    // parity: φ × φ = φ² → mod φ = 1 (toggle)
    
    vector<double> delta_sub(2, 0.0);
    delta_sub[0] = 1.0;    // EvalSub mag-subtract ng 1
    delta_sub[1] = 0.0;
    
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(delta_add);
    Plaintext pt_sub = cc->MakeCKKSPackedPlaintext(delta_sub);

    cout << "  Initial state:\n";
    auto v_init = decrypt_state(ct_state);
    cout << "    Log: " << v_init[0] << "\n";
    cout << "    Parity φ-power: " << v_init[1] << " (odd)\n\n";

    cout << "  Mixed operations:\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, pt_add);
        } else {
            ct_state = cc->EvalSub(ct_state, pt_sub);
        }
        
        if (i % 10 == 0 || i == 99) {
            auto v = decrypt_state(ct_state);
            cout << "  Step " << setw(2) << i << ": log=" << setw(8) << v[0] 
                 << ", parity_pow=" << setw(8) << v[1] << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final state:\n";
    cout << "    Log: " << v_final[0] << "\n";
    cout << "    Parity φ-power: " << v_final[1] << "\n";
    cout << "    φ^log: " << pow(PHI, v_final[0]) << "\n\n";
    
    cout << "  Expected log: 1 + 50×1 - 50×1 = 1\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
