// ============================================
// φ-8-BIT MULTIPLIER — REPEATED ADDITION
//
// Multiplier: A × B = A + A + ... (B times)
// Encrypted multiplication via repeated addition
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-8-BIT MULTIPLIER\n";
    cout << "========================================\n\n";

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

    const double PHI = 1.6180339887498948482;
    const double HALF_PHI = PHI / 2.0;
    const double TWO_PHI = 2.0 * PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Multiplier: repeated addition\n\n";

    auto encrypt_int = [&](int val) {
        vector<double> v(16, (double)val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // 8-BIT MULTIPLIER: 3 × 5 = 15
    // ============================================

    cout << "========================================\n";
    cout << "  8-BIT MULTIPLIER: 3 × 5\n";
    cout << "========================================\n\n";

    int A = 3;
    int B = 5;
    int expected = 15;

    auto ct_result = encrypt_int(0);
    auto ct_a = encrypt_int(A);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < B; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_a);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double avg = decrypt_avg(ct_result);
    int decoded = (int)round(avg);

    cout << "  " << A << " × " << B << " = " << decoded << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";

    // ============================================
    // LARGER: 12 × 13 = 156
    // ============================================

    cout << "========================================\n";
    cout << "  LARGER: 12 × 13\n";
    cout << "========================================\n\n";

    A = 12;
    B = 13;
    expected = 156;

    ct_result = encrypt_int(0);
    ct_a = encrypt_int(A);

    start = high_resolution_clock::now();

    for (int i = 0; i < B; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_a);
    }

    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();

    avg = decrypt_avg(ct_result);
    decoded = (int)round(avg);

    cout << "  " << A << " × " << B << " = " << decoded << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";

    return 0;
}
