// ============================================
// φ-MIXED OPS PERIODIC
// Subukan ang EvalSub at ibang combination
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

    auto encrypt_state = [&](double val) {
        vector<double> v(2, 0.0);
        v[0] = val;
        v[1] = fmod(val, PHI);  // φ-mod sa value
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
    cout << "  φ-MIXED OPS PERIODIC — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Mixed: EvalAdd at EvalSub na may φ-mod\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(1.0);
    auto ct_phi = encrypt_state(PHI);
    auto ct_one = encrypt_state(1.0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // Subukan ang ibang combination:
        // Kapag value > φ: EvalSub(φ) para i-reset
        // Kapag value < 0: EvalAdd(φ) para i-reset
        // Ito ay natural na φ-periodic na operasyon
        
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_one);
        } else {
            ct_state = cc->EvalSub(ct_state, ct_one);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final value: " << v_final[0] << "\n";
    cout << "  Final φ-mod: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
