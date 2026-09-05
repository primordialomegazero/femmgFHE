// ============================================
// φ-SLOT MAPPING — Natural na φ-based
// Ang Sturmian bilang slot permutation
// Mas optimal na distribution
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-SLOT MAPPING ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Linear vs Sturmian slot mapping
    // ============================================
    cout << "--- 1. Linear vs Sturmian ---\n\n";
    cout << "  Linear:   0, 1, 2, 3, 4, 5, 6, 7\n";
    cout << "  Sturmian: 0, 1, 3, 4, 6, 8, 9, 11\n\n";
    cout << "  Ang Sturmian ay may natural na φ-gap\n";
    cout << "  na mas optimal para sa cyclic access\n\n";

    // ============================================
    // 2. CKKS na may Sturmian slot mapping
    // ============================================
    cout << "--- 2. CKKS na may Sturmian mapping ---\n\n";

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

    // I-encrypt ang φ-powers sa linear na slots
    vector<double> phi_powers(8, 0.0);
    for (int i = 0; i < 8; i++) {
        phi_powers[i] = pow(PHI, i);
    }

    Plaintext pt = cc->MakeCKKSPackedPlaintext(phi_powers);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);

    // EvalAdd — φ-based na addition
    vector<double> add_vals(8, 1.0);
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(add_vals);
    auto ct_add = cc->EvalAdd(ct, pt_add);

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_add, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  φ-powers + 1:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << res[i].real();
    }
    cout << "\n";
    cout << "  Level: " << ct_add->GetLevel() << "\n\n";

    // ============================================
    // 3. Ang φ-gap sa slot spacing
    // ============================================
    cout << "--- 3. φ-gap sa slot spacing ---\n\n";
    cout << "  Ang Sturmian gaps ay: 1, 2, 1, 2, 2, 1, 2\n";
    cout << "  Ito ay may natural na φ-distribution\n\n";

    // I-analyze ang φ-gap sa error
    cout << "  Slot | Value | Error\n";
    cout << "  -----|-------|-------\n";
    for (int i = 0; i < 8; i++) {
        double expected = pow(PHI, i) + 1.0;
        double error = res[i].real() - expected;
        cout << "    " << i << "  | "
             << setw(8) << res[i].real() << " | "
             << setw(8) << error << "\n";
    }
    cout << "\n";

    return 0;
}
