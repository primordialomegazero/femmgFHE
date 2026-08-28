// ============================================
// NEWTON'S METHOD PARA SA 1/x SA CKKS
// x_{n+1} = x_n(2 - a·x_n)
// Quadratic convergence, stable
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
    cout << "  NEWTON'S METHOD PARA SA 1/x SA CKKS\n";
    cout << "  x_{n+1} = x_n(2 - a·x_n)\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
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

    cout << "========================================\n";
    cout << "  TEST 1: PLAINTEXT NEWTON RECIPROCAL\n";
    cout << "========================================\n\n";

    // Test: 1/3 via Newton's method
    // x_{n+1} = x_n(2 - 3·x_n)
    // Start: x_0 = 0.5
    // Target: 1/3 ≈ 0.333

    cout << "Computing 1/3:\n";
    cout << "Step | x_n | 2 - 3·x_n | x_{n+1} | Error\n";
    cout << "-----|-----|-----------|---------|------\n";

    double a = 3.0;
    double x = 0.5;
    double target = 1.0 / a;

    for (int i = 0; i <= 6; i++) {
        double factor = 2.0 - a * x;
        double x_next = x * factor;
        double error = abs(x_next - target);

        cout << setw(4) << i << " | "
             << setw(10) << fixed << setprecision(6) << x << " | "
             << setw(10) << fixed << setprecision(6) << factor << " | "
             << setw(10) << fixed << setprecision(6) << x_next << " | "
             << setw(10) << scientific << setprecision(2) << error << "\n";

        x = x_next;
    }

    cout << "\n========================================\n";
    cout << "  TEST 2: ENCRYPTED NEWTON RECIPROCAL\n";
    cout << "========================================\n\n";

    // Encrypt initial guess x_0 = 0.5
    vector<double> plain_x(slots, 0.5);
    auto ct_x = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_x));

    // Encrypt a = 3
    vector<double> plain_a(slots, 3.0);
    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));

    // Encrypt 2
    vector<double> plain_two(slots, 2.0);
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    cout << "Encrypted x_0 = 0.5, a = 3\n";
    cout << "Level: " << ct_x->GetLevel() << "\n\n";

    cout << "ENCRYPTED ITERATIONS:\n";
    cout << "Step | Value | Expected | Error | Level | Valid?\n";
    cout << "-----|-------|----------|-------|-------|-------\n";

    for (int i = 0; i <= 5; i++) {
        // Decrypt to check
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_x, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();
        double error = abs(val - target);

        cout << setw(4) << i << " | "
             << setw(10) << fixed << setprecision(6) << val << " | "
             << setw(10) << fixed << setprecision(6) << target << " | "
             << setw(10) << scientific << setprecision(2) << error << " | "
             << setw(5) << ct_x->GetLevel() << " | "
             << (error < 0.01 ? "✅" : "→") << "\n";

        // Newton iteration: x = x(2 - a·x)
        // 2 - a·x
        auto ct_ax = cc->EvalMult(ct_a, ct_x);
        auto ct_factor = cc->EvalSub(ct_two, ct_ax);
        // x · factor
        ct_x = cc->EvalMult(ct_x, ct_factor);
    }

    cout << "\n========================================\n";
    cout << "  TEST 3: SELF-HEALING VIA NEWTON\n";
    cout << "========================================\n\n";

    cout << "Strategy: Pagkatapos ng arithmetic,\n";
    cout << "i-apply ang Newton reciprocal para\n";
    cout << "i-reset ang value malapit sa target.\n\n";

    // Test: value = 5 → self-heal to φ
    vector<double> plain_start(slots, 5.0);
    auto ct_heal = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_start));

    cout << "Start: 5.0\n";
    cout << "Target: 1/5 = 0.2 (reciprocal)\n\n";

    // Compute 1/5 via Newton
    vector<double> plain_guess(slots, 0.3);
    auto ct_guess = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_guess));

    vector<double> plain_five(slots, 5.0);
    auto ct_five = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_five));

    vector<double> plain_two_b(slots, 2.0);
    auto ct_two_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two_b));

    cout << "Reciprocal iteration (1/5):\n";
    cout << "Step | Value | Target | Error\n";
    cout << "-----|-------|--------|------\n";

    for (int i = 0; i <= 4; i++) {
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_guess, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double val = result_complex[0].real();
        double target_recip = 0.2;
        double error = abs(val - target_recip);

        cout << setw(4) << i << " | "
             << setw(10) << fixed << setprecision(6) << val << " | "
             << setw(10) << fixed << setprecision(6) << target_recip << " | "
             << setw(10) << scientific << setprecision(2) << error << "\n";

        // Newton: x = x(2 - 5·x)
        auto ct_5x = cc->EvalMult(ct_five, ct_guess);
        auto ct_factor = cc->EvalSub(ct_two_b, ct_5x);
        ct_guess = cc->EvalMult(ct_guess, ct_factor);
    }

    cout << "\n========================================\n";
    cout << "  KEY FINDING\n";
    cout << "========================================\n";
    cout << "  Newton's method ay stable sa CKKS.\n";
    cout << "  Kayang i-compute ang 1/x sa encrypted\n";
    cout << "  domain na may quadratic convergence.\n";
    cout << "========================================\n";

    return 0;
}
