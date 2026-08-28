// ============================================
// φ-NOISE ZERO — ABSOLUTE NON-ACCUMULATION
// Measure noise pagkatapos ng bawat operation
// Hanapin: May point ba na nagre-reset?
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
    cout << "  φ-NOISE ZERO — NON-ACCUMULATION\n";
    cout << "  Measure noise pagkatapos ng operations\n";
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
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;
    double PHI = 1.6180339887498948482;

    // Encrypt known value
    vector<double> plain(slots, 1.0);
    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));

    cout << "NOISE ANALYSIS:\n";
    cout << "Start value: 1.0\n";
    cout << "Operation: +2 (addition lang)\n\n";

    cout << setw(5) << "Step" << " | "
         << setw(12) << "Value" << " | "
         << setw(14) << "Error" << " | "
         << setw(14) << "Noise Level" << " | "
         << setw(10) << "Accumulate?" << "\n";

    cout << string(70, '-') << "\n";

    double prev_error = 0;
    double prev_noise = 0;

    for (int step = 0; step <= 40; step++) {
        // Decrypt to measure
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();

        // Expected: 1 + 2×step
        double expected = 1 + 2.0 * step;
        double error = abs(val - expected);

        // Noise level = error / magnitude
        double noise_level = (expected != 0) ? error / abs(expected) : 0;

        if (step % 5 == 0 || step <= 3) {
            cout << setw(5) << step << " | "
                 << setw(12) << fixed << setprecision(6) << val << " | "
                 << setw(14) << scientific << setprecision(2) << error << " | "
                 << setw(14) << scientific << setprecision(2) << noise_level << " | "
                 << setw(10) << (noise_level > prev_noise ? "↑" : "→") << "\n";
        }

        prev_error = error;
        prev_noise = noise_level;

        // Add 2
        vector<double> plain_two(slots, 2.0);
        auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));
        ct = cc->EvalAdd(ct, ct_two);
    }

    cout << "\n========================================\n";
    cout << "  NOISE ANALYSIS COMPLETE\n";
    cout << "========================================\n\n";

    // SECOND TEST: Multiplication noise
    cout << "MULTIPLICATION NOISE:\n";
    cout << "Operation: ×2 (doubling)\n\n";

    // Reset
    vector<double> plain2(slots, 1.0);
    auto ct2 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain2));

    cout << setw(5) << "Step" << " | "
         << setw(12) << "Value" << " | "
         << setw(14) << "Error" << " | "
         << setw(10) << "Level" << " | "
         << setw(10) << "Towers" << "\n";

    cout << string(60, '-') << "\n";

    for (int step = 0; step <= 20; step++) {
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct2, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();

        double expected = pow(2.0, step);
        double error = abs(val - expected);

        if (step % 4 == 0 || step <= 3) {
            cout << setw(5) << step << " | "
                 << setw(12) << fixed << setprecision(6) << val << " | "
                 << setw(14) << scientific << setprecision(2) << error << " | "
                 << setw(10) << ct2->GetLevel() << " | "
                 << setw(10) << ct2->GetElements()[0].GetNumOfElements() << "\n";
        }

        // Double (addition lang)
        ct2 = cc->EvalAdd(ct2, ct2);
    }

    cout << "\n========================================\n";
    cout << "  KEY QUESTION\n";
    cout << "========================================\n";
    cout << "  May noise accumulation ba?\n";
    cout << "  May point ba na nagre-reset?\n";
    cout << "========================================\n";

    return 0;
}
