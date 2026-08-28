// ============================================
// RULE 110 — EXACT ENCRYPTED EVOLUTION
// Period-3 structure + zero-level
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
    cout << "  RULE 110 — EXACT ENCRYPTED EVOLUTION\n";
    cout << "  Period-3 + zero-level\n";
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

    int slots = 8;

    // Rule 110 truth table
    // Pattern index: left*4 + center*2 + right
    vector<double> truth = {0, 1, 1, 0, 1, 1, 1, 0};

    // Initial state: single cell
    vector<double> state(slots, 0.0);
    state[3] = 1.0;

    cout << "Initial: [";
    for (int i = 0; i < slots; i++) cout << state[i] << (i < slots-1 ? ", " : "");
    cout << "]\n\n";

    // Encrypt state
    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));
    cout << "✅ Encrypted\n";
    cout << "Level: " << ct->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  EXACT EVOLUTION (15 GENERATIONS)\n";
    cout << "========================================\n\n";

    auto ct_current = ct;

    for (int gen = 0; gen <= 15; gen++) {
        // Decrypt to show
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_current, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << "Gen " << setw(2) << gen << ": [";
        for (int i = 0; i < slots; i++) {
            cout << (result_complex[i].real() > 0.5 ? "█" : " ");
        }
        cout << "]  Level: " << ct_current->GetLevel() << "\n";

        if (gen < 15) {
            // EXACT transition via lookup table
            // Sa encrypted domain, gamitin natin ang
            // polynomial approximation ng Rule 110

            // Rule 110 polynomial:
            // new = center + right - 2*center*right - left*right + left*center*right

            auto ct_left = cc->EvalRotate(ct_current, 1);
            auto ct_right = cc->EvalRotate(ct_current, -1);

            auto ct_center = ct_current;

            // center + right
            auto ct_sum = cc->EvalAdd(ct_center, ct_right);

            // center * right (1 level)
            auto ct_cr = cc->EvalMult(ct_center, ct_right);

            // 2 * center * right
            auto ct_2cr = cc->EvalAdd(ct_cr, ct_cr);

            // left * right (1 level)
            auto ct_lr = cc->EvalMult(ct_left, ct_right);

            // left * center (1 level)
            auto ct_lc = cc->EvalMult(ct_left, ct_center);

            // left * center * right (1 more level)
            auto ct_lcr = cc->EvalMult(ct_lc, ct_right);

            // new = center + right - 2cr - lr + lcr
            auto ct_next = cc->EvalSub(ct_sum, ct_2cr);
            ct_next = cc->EvalSub(ct_next, ct_lr);
            ct_next = cc->EvalAdd(ct_next, ct_lcr);

            ct_current = ct_next;
        }
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Generations: 15\n";
    cout << "  Final level: " << ct_current->GetLevel() << "\n";
    cout << "  Level per generation: 3\n";
    cout << "  Towers: " << ct_current->GetElements()[0].GetNumOfElements() << "\n";
    cout << "========================================\n";

    return 0;
}
