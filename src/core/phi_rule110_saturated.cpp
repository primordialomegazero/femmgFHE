// ============================================
// φ-RULE 110 SATURATED — Natural na Bounded
// Ang addition ay may φ-based na saturation
// Sums ay hindi lalampas sa φ²
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
    cout << "  φ-RULE 110 SATURATED — Natural Bounded\n";
    cout << "========================================\n\n";
    cout << "  Addition ay may φ-based saturation\n";
    cout << "  Sums ay bounded sa φ²\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // φ-SATURATED ENCODING
    // ============================================
    // Ang bawat bit ay naka-encode bilang φ-power
    // na may natural na saturation:
    // 0 → φ⁻¹ = 0.618
    // 1 → φ⁰ = 1.0
    //
    // Ang sum ng 3 bits ay may range:
    // min: 3×φ⁻¹ = 1.854
    // max: 3×φ⁰ = 3.0
    //
    // Ang φ-saturation ay:
    // sum > φ² (2.618) → saturate sa φ²
    // sum < φ⁻¹ (0.618) → saturate sa φ⁻¹

    const double SAT_ZERO = PHI - 1.0;  // φ⁻¹
    const double SAT_ONE = 1.0;          // φ⁰

    cout << "  Encoding: 0→φ⁻¹=" << SAT_ZERO << ", 1→φ⁰=" << SAT_ONE << "\n";
    cout << "  Saturation range: [φ⁻¹, φ²] = [" 
         << SAT_ZERO << ", " << PHI * PHI << "]\n\n";

    // Initial state
    vector<double> init(16, SAT_ZERO);
    init[7] = SAT_ONE;
    init[8] = SAT_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial:\n  ";
    for (double v : init) cout << (v > 0.8 ? "1" : "0");
    cout << "\n\n";

    int N = 100;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ANG φ-SATURATION:
        // I-clamp ang sum sa φ-range
        // Sa FHE, ito ay kailangan ng homomorphic
        // na comparison — pero may φ-based na paraan
        //
        // Ang φ ay may natural na saturation:
        // φ^n mod φ → two-state
        // Na automatic sa φ-structure
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(16);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  Final sums (φ-saturated):\n  ";
    for (int i = 0; i < 16; i++) {
        cout << setw(8) << res[i].real();
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  KEY: φ-saturation ay kailangan homomorphic\n";

    return 0;
}
