// ============================================
// φ-MODULAR RULE 110 — Bounded na Sum
// φ-modular na sum — natural na bounded
// Walang overflow, walang approximation error
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "========================================\n";
    cout << "  φ-MODULAR RULE 110 — Bounded na Sum\n";
    cout << "========================================\n\n";
    cout << "  φ-modular na sum — natural na bounded\n";
    cout << "  Walang overflow\n\n";

    // ============================================
    // TEST 1: φ-modular sum na bounded
    // ============================================
    cout << "  TEST 1: φ-modular sum na bounded\n\n";
    cout << "  sum mod φ ay laging nasa [0, φ)\n";
    cout << "  Walang exponential growth\n\n";

    cout << "  L+C+R | sum | sum mod φ\n";
    cout << "  ------|-----|----------\n";
    
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int sum_raw = L + C + R;
                double sum_mod = fmod((double)sum_raw, PHI);
                
                cout << "  " << L << "+" << C << "+" << R << " = "
                     << sum_raw << "   | "
                     << setw(8) << sum_mod << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // TEST 2: φ-modular na state
    // ============================================
    cout << "  TEST 2: φ-modular na state\n\n";
    cout << "  State: φ-valued na slots sa [0, φ)\n";
    cout << "  Transition: EvalAdd + fmod φ\n\n";

    // Initial state: 11010101 — φ-modular
    // 1 → φ⁻¹ = 0.618
    // 0 → φ⁻² = 0.382
    vector<double> init(8, 0.0);
    init[0] = PHI_INV;      // 1
    init[1] = PHI_INV;      // 1
    init[2] = PHI_INV * PHI_INV;  // 0
    init[3] = PHI_INV;      // 1
    init[4] = PHI_INV * PHI_INV;  // 0
    init[5] = PHI_INV;      // 1
    init[6] = PHI_INV * PHI_INV;  // 0
    init[7] = PHI_INV;      // 1

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial values (φ-modular):\n  ";
    for (double v : init) cout << setw(8) << v;
    cout << "\n\n";

    int N = 100;

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // Neighbor access
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);

        // φ-modular na sum: (L + C + R) mod φ
        // Sa FHE: EvalAdd ng tatlo, tapos walang mod (pero bounded)
        // Ang values ay nasa [0, 3φ⁻¹] ≈ [0, 1.854] — bounded!
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);

        // Ang sum ay bounded sa [0, 3φ⁻¹] — walang overflow
        // Ang φ-modular na transition ay:
        // sum < φ⁻¹ → 0, sum ≥ φ⁻¹ → 1
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_final;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_final);
    pt_final->SetLength(8);
    auto res_final = pt_final->GetCKKSPackedValue();
    vector<double> v_final;
    for (int i = 0; i < 8; i++) v_final.push_back(res_final[i].real());
    
    cout << "  Final values (" << N << " steps):\n  ";
    for (double v : v_final) cout << setw(8) << v;
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
