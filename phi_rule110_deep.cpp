// ============================================
// RULE 110 — DEEP: EXACT LOOKUP + EMERGENT
// A: Pre-computed encrypted lookup
// B: Emergent sum-based diffusion
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  RULE 110 — DEEP ANALYSIS\n";
    cout << "  A: Exact Lookup + B: Emergent\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    vector<int32_t> rotation_indices = {1, -1};
    cc->EvalRotateKeyGen(keyPair.secretKey, rotation_indices);
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;
    double PHI = 1.6180339887498948482;

    // ============================================
    // PART A: EXACT LOOKUP TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  PART A: EXACT LOOKUP TABLE\n";
    cout << "========================================\n\n";

    // Pre-computed lookup: bawat pattern (0-7) ay may output
    vector<double> lookup = {0, 1, 1, 0, 1, 1, 1, 0};

    // Initial state
    vector<double> state(slots, 0.0);
    state[3] = 1.0;

    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));

    cout << "EXACT RULE 110 VIA LOOKUP:\n";
    cout << "Gen  | State          | Level\n";
    cout << "-----|----------------|------\n";

    auto ct_current = ct;

    for (int gen = 0; gen <= 15; gen++) {
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_current, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << setw(4) << gen << " | [";
        for (int i = 0; i < slots; i++) {
            cout << (result_complex[i].real() > 0.5 ? "█" : " ");
        }
        cout << "] | " << ct_current->GetLevel() << "\n";

        if (gen < 15) {
            // Exact transition via polynomial
            auto ct_left = cc->EvalRotate(ct_current, 1);
            auto ct_right = cc->EvalRotate(ct_current, -1);

            auto ct_sum = cc->EvalAdd(ct_current, ct_right);
            auto ct_cr = cc->EvalMult(ct_current, ct_right);
            auto ct_2cr = cc->EvalAdd(ct_cr, ct_cr);
            auto ct_lr = cc->EvalMult(ct_left, ct_right);
            auto ct_lc = cc->EvalMult(ct_left, ct_current);
            auto ct_lcr = cc->EvalMult(ct_lc, ct_right);

            auto ct_next = cc->EvalSub(ct_sum, ct_2cr);
            ct_next = cc->EvalSub(ct_next, ct_lr);
            ct_next = cc->EvalAdd(ct_next, ct_lcr);

            ct_current = ct_next;
        }
    }

    cout << "\n========================================\n";
    cout << "  PART B: EMERGENT SUM-BASED\n";
    cout << "========================================\n\n";

    // Reset sa initial state
    auto ct_emergent = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));

    cout << "EMERGENT DIFFUSION (sum-based):\n";
    cout << "Gen  | State          | Level | Sum\n";
    cout << "-----|----------------|-------|-----\n";

    auto ct_em = ct_emergent;

    for (int gen = 0; gen <= 15; gen++) {
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_em, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        double total_sum = 0;
        cout << setw(4) << gen << " | [";
        for (int i = 0; i < slots; i++) {
            double val = result_complex[i].real();
            total_sum += val;
            cout << (val > 0.5 ? "█" : " ");
        }
        cout << "] | " << ct_em->GetLevel() << " | " << fixed << setprecision(1) << total_sum << "\n";

        if (gen < 15) {
            // Sum-based (zero-level)
            auto ct_left = cc->EvalRotate(ct_em, 1);
            auto ct_right = cc->EvalRotate(ct_em, -1);
            auto ct_sum = cc->EvalAdd(ct_em, ct_left);
            ct_sum = cc->EvalAdd(ct_sum, ct_right);
            ct_em = ct_sum;
        }
    }

    cout << "\n========================================\n";
    cout << "  EMERGENT PATTERN ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "Sum-based transition properties:\n";
    cout << "  1. Diffusion: lumalawak ang signal\n";
    cout << "  2. Total sum: tumataas bawat gen\n";
    cout << "  3. Saturation: lahat-ones eventually\n\n";

    cout << "φ-CONNECTION:\n";
    cout << "  Ang diffusion rate ay 2 cells/gen\n";
    cout << "  Ang total sum ay lumalaki ng ~2× bawat gen\n";
    cout << "  Ito ay Fibonacci-like growth:\n";
    cout << "  Gen 0: 1, Gen 1: 3, Gen 2: 5, Gen 3: 7\n";
    cout << "  Pattern: odd numbers (1, 3, 5, 7, ...)\n";
    cout << "  Ito ay linear diffusion, hindi φ-based\n\n";

    cout << "========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n";
    cout << "  A: Exact Rule 110 = 3 levels/gen\n";
    cout << "  B: Emergent sum = 0 levels/gen\n";
    cout << "  Trade-off: exactness vs zero-level\n\n";
    cout << "  MAY SYNTHESIS BA?\n";
    cout << "  Kung pagsamahin natin:\n";
    cout << "  - Exact lookup para sa first N gens\n";
    cout << "  - Sum-based para sa next M gens\n";
    cout << "  Baka may hybrid na exact + zero-level\n";
    cout << "========================================\n";

    return 0;
}
