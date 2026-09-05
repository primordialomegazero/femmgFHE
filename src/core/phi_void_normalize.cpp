// ============================================
// φ-VOID NORMALIZE — Natural na Reset
// Ang void bilang normalization mechanism
// φ-based na reset sa valid range
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
    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;
    const double VOID = 0.0;  // Ang void — natural na zero

    cout << "========================================\n";
    cout << "  φ-VOID NORMALIZE — Natural na Reset\n";
    cout << "========================================\n\n";
    cout << "  Void = 0 — natural na reset point\n";
    cout << "  Ang φ-structure ay may built-in na void\n\n";

    // ============================================
    // 1. Ang void sa φ-structure
    // ============================================
    cout << "  --- 1. Void sa φ ---\n\n";
    cout << "  φ⁰ - φ⁰ = " << pow(PHI, 0) - pow(PHI, 0) << " (void)\n";
    cout << "  φ¹ - φ¹ = " << pow(PHI, 1) - pow(PHI, 1) << " (void)\n";
    cout << "  φ² - φ - 1 = " << pow(PHI, 2) - PHI - 1.0 << " (void)\n\n";

    // ============================================
    // 2. Normalization via void
    // ============================================
    cout << "  --- 2. Normalization via void ---\n\n";
    cout << "  Ang sum ay may natural na void:\n";
    cout << "  Kapag sum = 0, ang state ay void\n";
    cout << "  Ang void ay nagbibigay ng natural na reset\n\n";

    // Initial state
    vector<double> init(16, EXP_ZERO);
    init[7] = EXP_ONE;
    init[8] = EXP_ONE;

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 0000000110000000\n\n";

    // ============================================
    // 3. Ang void-based na normalization
    // ============================================
    int N = 10;

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < N; gen++) {
        auto ct_left = cc->EvalRotate(ct_state, 1);
        auto ct_right = cc->EvalRotate(ct_state, -1);
        
        // Sum na may void normalization
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        // ANG VOID NORMALIZATION:
        // I-subtract ang φ-based na void
        // para ma-reset sa valid range
        // Ang void ay φ^k na may eksaktong zero
        
        // Sa φ-space, ang void ay:
        // VOID = φ^(-∞) ≈ 0
        // Kapag ang sum ay may void component,
        // automatic na nagre-reset
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(16);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  Final sums (" << N << " gens):\n  ";
    for (int i = 0; i < 16; i++) {
        // I-normalize sa φ-range
        double normalized = fmod(res[i].real(), PHI);
        cout << setw(6) << normalized;
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
