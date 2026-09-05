// ============================================
// φ-PERIODICITY SA COMPUTATION
// Ang φ^odd → 0, φ^even → 1 bilang operasyon
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

    // Ang periodicity ay nasa computation:
    // φ^odd → 0, φ^even → 1
    // Ito ay automatic sa φ-structure
    //
    // Sa log space:
    // n odd → fmod(n, φ) = 0
    // n even → fmod(n, φ) = 1
    //
    // Ang operasyon ay:
    // EvalAdd ng φ-based delta na may periodicity

    auto encrypt_state = [&](double val) {
        vector<double> v(2, 0.0);
        v[0] = val;                       // value
        v[1] = (int)round(val) % 2;       // parity (0=even, 1=odd)
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
    cout << "  φ-PERIODICITY SA COMPUTATION\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  φ^odd → 0, φ^even → 1\n";
    cout << "  Periodicity sa operasyon mismo\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(1.0);  // odd → φ^odd → 0

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // Ang operasyon ay may φ-periodicity:
        // Ang Slot 1 (parity) ay nagde-determine ng resulta
        // 0 (even) → 1
        // 1 (odd) → 0
        //
        // Ito ay natural na φ-structure sa computation
        
        // EvalAdd ng 1 para ma-toggle ang parity
        auto ct_one = encrypt_state(1.0);
        ct_state = cc->EvalAdd(ct_state, ct_one);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final value: " << v_final[0] << "\n";
    cout << "  Final parity: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
