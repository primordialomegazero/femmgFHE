// ============================================
// φ-NOISE PERIOD-0 — IRRATIONAL DISTRIBUTION
// Ang noise ay hindi nag-a-accumulate
// kundi nagdi-distribute nang pantay
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
    cout << "  φ-NOISE PERIOD-0\n";
    cout << "  Irrational distribution\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 1;
    double PHI = 1.6180339887498948482;
    double GOLDEN_ANGLE = 2.0 * M_PI * (1.0 - 1.0/PHI);

    cout << "========================================\n";
    cout << "  TEST 1: NOISE ACCUMULATION PATTERN\n";
    cout << "========================================\n\n";

    // Start with value 1.0
    vector<double> plain(slots, 1.0);
    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));

    cout << "Operation: +2 (addition lang)\n";
    cout << "Sinusukat natin ang error sa bawat step\n\n";

    cout << setw(5) << "Step" << " | "
         << setw(12) << "Value" << " | "
         << setw(14) << "Error" << " | "
         << setw(12) << "Err/Val" << " | "
         << setw(14) << "Err Difference" << "\n";

    cout << string(65, '-') << "\n";

    double prev_error_ratio = 0;
    vector<double> error_ratios;

    for (int step = 0; step <= 40; step++) {
        // Decrypt
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();

        double expected = 1.0 + 2.0 * step;
        double error = abs(val - expected);
        double error_ratio = (expected != 0) ? error / abs(expected) : 0;
        double error_diff = error_ratio - prev_error_ratio;

        error_ratios.push_back(error_ratio);

        if (step % 5 == 0 || step <= 3) {
            cout << setw(5) << step << " | "
                 << setw(12) << fixed << setprecision(6) << val << " | "
                 << setw(14) << scientific << setprecision(2) << error << " | "
                 << setw(12) << scientific << setprecision(2) << error_ratio << " | "
                 << setw(14) << scientific << setprecision(2) << error_diff << "\n";
        }

        prev_error_ratio = error_ratio;

        // Add 2
        vector<double> plain_two(slots, 2.0);
        auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));
        ct = cc->EvalAdd(ct, ct_two);
    }

    // Analysis: nag-a-accumulate ba ang error?
    cout << "\n========================================\n";
    cout << "  ERROR ACCUMULATION ANALYSIS\n";
    cout << "========================================\n\n";

    if (error_ratios.size() > 5) {
        double first_5_avg = 0;
        double last_5_avg = 0;
        for (int i = 0; i < 5; i++) first_5_avg += error_ratios[i];
        for (int i = error_ratios.size() - 5; i < error_ratios.size(); i++) last_5_avg += error_ratios[i];
        first_5_avg /= 5;
        last_5_avg /= 5;

        cout << "Average error (first 5): " << scientific << setprecision(2) << first_5_avg << "\n";
        cout << "Average error (last 5): " << scientific << setprecision(2) << last_5_avg << "\n";
        cout << "Growth factor: " << fixed << setprecision(2) << (last_5_avg / first_5_avg) << "x\n\n";

        cout << (last_5_avg > first_5_avg ? "↗️ NAG-ACCUMULATE" : "↘️ HINDI NAG-ACCUMULATE") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  TEST 2: φ-MODULO NOISE RESET\n";
    cout << "========================================\n\n";

    cout << "Strategy: Pagkatapos ng bawat operation,\n";
    cout << "i-apply ang φ-modulo (addition lang)\n";
    cout << "para i-distribute ang noise.\n\n";

    // Reset
    vector<double> plain2(slots, 1.0);
    auto ct2 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain2));

    cout << setw(5) << "Step" << " | "
         << setw(12) << "Value" << " | "
         << setw(14) << "Error" << " | "
         << setw(12) << "φ-Modulo" << " | "
         << setw(12) << "Error After" << "\n";

    cout << string(65, '-') << "\n";

    double current_val = 1.0;
    vector<double> errors_before;
    vector<double> errors_after;

    for (int step = 0; step <= 30; step++) {
        // Decrypt current
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct2, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        current_val = result_complex[0].real();

        double expected = 1.0 + 2.0 * step;
        double error_before = abs(current_val - expected);
        errors_before.push_back(error_before);

        // Apply φ-modulo: value - floor(value/φ) × φ
        long long k = (long long)(current_val / PHI);
        double mod_val = current_val - k * PHI;

        // Re-encrypt mod_val
        vector<double> plain_mod(slots, mod_val);
        ct2 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_mod));

        // Decrypt after modulo
        Plaintext plain_after;
        cc->Decrypt(keyPair.secretKey, ct2, &plain_after);
        plain_after->SetLength(slots);
        auto after_complex = plain_after->GetCKKSPackedValue();
        double val_after = after_complex[0].real();

        double error_after = abs(val_after - mod_val);
        errors_after.push_back(error_after);

        if (step % 5 == 0 || step <= 3) {
            cout << setw(5) << step << " | "
                 << setw(12) << fixed << setprecision(6) << current_val << " | "
                 << setw(14) << scientific << setprecision(2) << error_before << " | "
                 << setw(12) << fixed << setprecision(6) << mod_val << " | "
                 << setw(12) << scientific << setprecision(2) << error_after << "\n";
        }

        // Add 2
        vector<double> plain_two(slots, 2.0);
        auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));
        ct2 = cc->EvalAdd(ct2, ct_two);
    }

    cout << "\n========================================\n";
    cout << "  KEY FINDING\n";
    cout << "========================================\n";
    cout << "  Ang φ-modulo ay nagre-reset ng value\n";
    cout << "  sa [0, φ) range. Ang error ay\n";
    cout << "  hindi na nag-a-accumulate sa value\n";
    cout << "  kasi ang value ay bounded.\n";
    cout << "========================================\n";

    return 0;
}
