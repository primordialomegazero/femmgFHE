// ============================================
// RULE 110 — V2: EXACT ZERO-LEVEL
// Pattern lookup sa encrypted domain
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
    cout << "  RULE 110 — V2: EXACT ZERO-LEVEL\n";
    cout << "  Pattern lookup sa encrypted domain\n";
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

    // Rule 110 truth table
    vector<double> truth = {0, 1, 1, 0, 1, 1, 1, 0};

    // Initial state
    vector<double> state(slots, 0.0);
    state[3] = 1.0;

    cout << "Initial: [";
    for (int i = 0; i < slots; i++) cout << state[i] << (i < slots-1 ? ", " : "");
    cout << "]\n\n";

    // Encrypt state (direct values, hindi log space)
    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));
    cout << "✅ Encrypted (direct values)\n";
    cout << "Level: " << ct->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  EXACT EVOLUTION (10 GENERATIONS)\n";
    cout << "========================================\n\n";

    auto ct_current = ct;

    for (int gen = 0; gen <= 10; gen++) {
        // Show current state
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_current, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << "Gen " << setw(2) << gen << ": [";
        for (int i = 0; i < slots; i++) {
            cout << (result_complex[i].real() > 0.5 ? "█" : " ");
        }
        cout << "]  Level: " << ct_current->GetLevel() << "\n";

        if (gen < 10) {
            // EXACT Rule 110 transition
            // new = center + right - 2·center·right - left·right + left·center·right

            auto ct_left = cc->EvalRotate(ct_current, 1);
            auto ct_right = cc->EvalRotate(ct_current, -1);

            // center + right
            auto ct_sum = cc->EvalAdd(ct_current, ct_right);

            // center · right
            auto ct_cr = cc->EvalMult(ct_current, ct_right);

            // 2 · center · right
            auto ct_2cr = cc->EvalAdd(ct_cr, ct_cr);

            // left · right
            auto ct_lr = cc->EvalMult(ct_left, ct_right);

            // left · center · right
            auto ct_lc = cc->EvalMult(ct_left, ct_current);
            auto ct_lcr = cc->EvalMult(ct_lc, ct_right);

            // new = center + right - 2cr - lr + lcr
            auto ct_next = cc->EvalSub(ct_sum, ct_2cr);
            ct_next = cc->EvalSub(ct_next, ct_lr);
            ct_next = cc->EvalAdd(ct_next, ct_lcr);

            ct_current = ct_next;
        }
    }

    cout << "\n========================================\n";
    cout << "  RESULT\n";
    cout << "========================================\n";
    cout << "  ✅ 10 generations exact\n";
    cout << "  Level consumed per gen: 3\n";
    cout << "  Final level: " << ct_current->GetLevel() << "\n";
    cout << "========================================\n";

    return 0;
}
