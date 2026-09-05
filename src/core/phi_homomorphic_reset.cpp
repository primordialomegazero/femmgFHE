// ============================================
// φ-HOMOMORPHIC RESET — 100 iterations
// Periodic reset sa anchor φ^n mod φ
// Walang decrypt, walang EvalMult
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
    // Slot 0: log value
    // Slot 1: value (φ^log)
    // Slot 2: reset counter
    // Slot 3: spare

    auto encrypt_state = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[0] = log_val;
        v[1] = pow(PHI, log_val);
        v[2] = 0.0;
        v[3] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-HOMOMORPHIC RESET — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Periodic reset sa anchor\n";
    cout << "  Walang decrypt, walang EvalMult\n";
    cout << "  Running...\n\n";

    // Start sa anchor: log=1 (value=φ)
    auto ct_state = encrypt_state(1.0);

    vector<double> delta_mul(4, 0.0);
    delta_mul[0] = 1.0;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(delta_mul);

    vector<double> delta_reset(4, 0.0);
    delta_reset[0] = -PHI;
    delta_reset[2] = 1.0;
    Plaintext pt_reset = cc->MakeCKKSPackedPlaintext(delta_reset);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        ct_state = cc->EvalAdd(ct_state, pt_mul);
        
        if (i % 5 == 4) {
            ct_state = cc->EvalAdd(ct_state, pt_reset);
        }
        
        if (i < 20 || i % 20 == 19) {
            auto v = decrypt_state(ct_state);
            cout << "  Step " << setw(3) << i << ": log=" << setw(10) << v[0]
                 << ", value=" << setw(12) << v[1]
                 << ", resets=" << setw(4) << v[2] << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final state:\n";
    cout << "    Log: " << v_final[0] << "\n";
    cout << "    Value: " << v_final[1] << "\n";
    cout << "    Resets: " << v_final[2] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
