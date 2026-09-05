// ============================================
// φ-LOG MIXED — 100 iterations
// Log space primary
// EvalAdd = multiply, EvalSub = divide
// Mixed: add, sub, mul, div sa iisang space
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
        v[0] = log_val;                 // log space (primary)
        v[1] = pow(PHI, log_val);       // normal space (reference)
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
    cout << "  φ-LOG MIXED — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Log space primary\n";
    cout << "  EvalAdd = multiply sa normal\n";
    cout << "  EvalSub = divide sa normal\n";
    cout << "  Running...\n\n";

    // Start sa log value = 1 (φ^1)
    auto ct_state = encrypt_log(1.0);

    // Mixed deltas sa log space:
    // +1 = multiply ng φ
    // -1 = divide ng φ
    // +log_φ(2) = multiply ng 2
    // -log_φ(3) = divide ng 3
    
    double delta_mul = 1.0;                    // ×φ
    double delta_div = -1.0;                   // ÷φ
    double delta_mul2 = log(2.0) / LN_PHI;     // ×2
    double delta_div3 = -log(3.0) / LN_PHI;    // ÷3

    vector<double> deltas = {delta_mul, delta_div, delta_mul2, delta_div3};
    
    vector<double> delta_v(2, 0.0);
    Plaintext pt_delta;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        delta_v[0] = deltas[i % 4];
        delta_v[1] = 0.0;
        pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Final normal: " << v_final[1] << "\n";
    cout << "  Expected log: " << 1.0 + 25.0*(delta_mul + delta_div + delta_mul2 + delta_div3) << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
