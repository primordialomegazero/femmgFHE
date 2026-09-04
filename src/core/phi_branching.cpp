// ============================================
// φ-BRANCHING — DATA-DEPENDENT
// Ang susunod na operasyon ay nakadepende sa value
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

    auto encrypt_log = [&](double log_val) {
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = pow(PHI, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-BRANCHING — DATA-DEPENDENT\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Branching: kung log > threshold, ×φ; kung hindi, ÷φ\n";
    cout << "  Data-dependent — hindi fixed cycle\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_log(3.0);
    
    // Deltas
    auto ct_mul = encrypt_log(1.0);
    auto ct_div = encrypt_log(-1.0);
    auto ct_add = encrypt_log(0.5);
    auto ct_sub = encrypt_log(-0.3);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // Sa totoong FHE, hindi natin mababasa ang value
        // Pero para sa demo, i-decrypt natin para sa branching decision
        auto v = decrypt_log(ct_state);
        double current_log = v[0];
        
        // Data-dependent branching
        if (current_log > 5.0) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
        } else if (current_log < 2.0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_sub);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
