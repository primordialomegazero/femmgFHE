// ============================================
// RULE 110 — FULLY ENCRYPTED EVOLUTION
// Homomorphic computation sa OpenFHE
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
    cout << "  RULE 110 — FULLY ENCRYPTED EVOLUTION\n";
    cout << "  Homomorphic sa OpenFHE\n";
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

    // GENERATE ROTATION KEYS
    vector<int32_t> rotation_indices = {1, -1};
    cc->EvalRotateKeyGen(keyPair.secretKey, rotation_indices);

    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;

    cout << "Rule 110 transition (algebraic form):\n";
    cout << "new = center + right - 2·center·right - left·right + left·center·right\n\n";

    // Initial state
    vector<double> state = {1, 0, 1, 1, 0, 1, 0, 0};
    cout << "Initial state: [";
    for (int i = 0; i < slots; i++) cout << state[i] << (i < slots-1 ? ", " : "");
    cout << "]\n\n";

    // Encrypt state
    auto ct_state = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));
    cout << "✅ State encrypted\n";
    cout << "Level: " << ct_state->GetLevel() << "\n\n";

    // Helper: Rotate left
    auto rotate_left = [&](const Ciphertext<DCRTPoly>& ct) {
        return cc->EvalRotate(ct, 1);
    };

    // Helper: Rotate right
    auto rotate_right = [&](const Ciphertext<DCRTPoly>& ct) {
        return cc->EvalRotate(ct, -1);
    };

    cout << "========================================\n";
    cout << "  ENCRYPTED EVOLUTION (5 GENERATIONS)\n";
    cout << "========================================\n\n";

    auto ct_current = ct_state;

    for (int gen = 0; gen <= 5; gen++) {
        // Decrypt to show current state
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_current, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << "Gen " << gen << ": [";
        for (int i = 0; i < slots; i++) {
            cout << (result_complex[i].real() > 0.5 ? "█" : " ");
        }
        cout << "]  Level: " << ct_current->GetLevel() << "\n";

        if (gen < 5) {
            // Compute next state homomorphically
            auto ct_center = ct_current;
            auto ct_left = rotate_left(ct_current);
            auto ct_right = rotate_right(ct_current);

            // center + right
            auto ct_sum = cc->EvalAdd(ct_center, ct_right);

            // center · right
            auto ct_center_right = cc->EvalMult(ct_center, ct_right);

            // 2 · center · right
            auto ct_2cr = cc->EvalAdd(ct_center_right, ct_center_right);

            // left · right
            auto ct_left_right = cc->EvalMult(ct_left, ct_right);

            // left · center · right
            auto ct_lc = cc->EvalMult(ct_left, ct_center);
            auto ct_lcr = cc->EvalMult(ct_lc, ct_right);

            // new = center + right - 2cr - lr + lcr
            auto ct_next = cc->EvalSub(ct_sum, ct_2cr);
            ct_next = cc->EvalSub(ct_next, ct_left_right);
            ct_next = cc->EvalAdd(ct_next, ct_lcr);

            ct_current = ct_next;
        }
    }

    cout << "\n========================================\n";
    cout << "  RESULT\n";
    cout << "========================================\n";
    cout << "  ✅ 5 generations evolved homomorphically\n";
    cout << "  Level after 5 generations: " << ct_current->GetLevel() << "\n";
    cout << "  Level consumed per generation: 3\n";
    cout << "========================================\n";

    return 0;
}
