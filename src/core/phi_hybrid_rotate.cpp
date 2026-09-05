// ============================================
// φ-HYBRID ROTATE
// φ-structure + EvalRotate para sa scale-aware
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1, 2, -2});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double n, double F) {
        vector<double> v(4, 0.0);
        v[0] = n;       // log space
        v[1] = F;       // normal space
        v[2] = n + 1;   // shifted log
        v[3] = F * PHI; // shifted normal
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
    cout << "  φ-HYBRID ROTATE — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  φ-structure + EvalRotate\n";
    cout << "  Scale-aware na operasyon\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(3.0, pow(PHI, 3.0));

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // HYBRID NA OPERASYON:
        // Ang state ay may 4 slots na may φ-structure
        // Slot 0: n (log)
        // Slot 1: F (normal)
        // Slot 2: n+1 (shifted log)
        // Slot 3: F×φ (shifted normal)
        //
        // Ang scale-aware na operasyon ay:
        // Kung F ay malapit sa φ^n, gamitin ang Slot 0
        // Kung F ay malapit sa φ^(n+1), gamitin ang Slot 2
        //
        // Ang EvalRotate ay nagbibigay ng access
        // sa shifted na slots nang walang decrypt
        
        // Para sa ngayon, simple lang:
        // EvalAdd ng rotated state para sa scale-aware
        auto ct_rot = cc->EvalRotate(ct_state, 1);
        ct_state = cc->EvalAdd(ct_state, ct_rot);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";
    cout << "  Final Slot 0 (n): " << v_final[0] << "\n";
    cout << "  Final Slot 1 (F): " << v_final[1] << "\n";
    cout << "  Final Slot 2: " << v_final[2] << "\n";
    cout << "  Final Slot 3: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
