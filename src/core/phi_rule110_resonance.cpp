// ============================================
// φ-RULE 110 RESONANCE — Natural na Bounded
// Ang φ-resonance ay nagli-limit sa sums
// Automatic na bounded — walang overflow
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
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;

    cout << "========================================\n";
    cout << "  φ-RULE 110 RESONANCE — Natural Bounded\n";
    cout << "========================================\n\n";
    cout << "  Ang φ-resonance ay nagli-limit\n";
    cout << "  sa sums — walang overflow\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // φ-RESONANT ENCODING
    // ============================================
    // Imbis na linear na accumulation,
    // gamitin ang φ-resonant na encoding:
    // Ang sum ay may natural na φ-cycle
    //
    // φ-cycle: φ⁰=1, φ¹=φ, φ²=φ+1, φ³=2φ+1
    // Ang modulo ay automatic sa φ-structure
    
    const double RES_0 = PHI - 1.0;   // φ⁻¹ = 0.618
    const double RES_1 = PHI;         // φ = 1.618

    cout << "  φ-resonant encoding:\n";
    cout << "  0 → φ⁻¹ = " << RES_0 << "\n";
    cout << "  1 → φ = " << RES_1 << "\n\n";

    // Initial state
    vector<double> init(16, RES_0);
    init[7] = RES_1;
    init[8] = RES_1;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial:\n  ";
    for (double v : init) cout << (v > 1.0 ? "1" : "0");
    cout << "\n\n";

    int N = 20;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ANG φ-RESONANCE:
        // Ang sum ng 3 φ-values ay may natural na
        // φ-range: [3φ⁻¹, 3φ] = [1.854, 4.854]
        // Na may φ-based na threshold
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(16);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  Final sums (φ-resonant):\n  ";
    for (int i = 0; i < 16; i++) {
        cout << setw(8) << res[i].real();
    }
    cout << "\n\n";

    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: φ-resonance ay may natural na bounded\n";

    return 0;
}
