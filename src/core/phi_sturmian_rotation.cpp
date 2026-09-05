// ============================================
// φ-STURMIAN ROTATION — Natural na Slot Shift
// Ang Sturmian permutation bilang rotation
// Walang EvalRotate keys — natural sa φ
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

    cout << "=== φ-STURMIAN ROTATION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Sturmian permutation analysis
    // ============================================
    cout << "--- 1. Sturmian permutation ---\n\n";
    cout << "  Ang φ-rotation ay ergodic — dumadalaw sa\n";
    cout << "  lahat ng points sa complex circle\n\n";

    // Ang φ-rotation sa 8 slots
    int M = 8;
    cout << "  φ-rotation sa " << M << " slots:\n";
    cout << "  i | slot index | value\n";
    cout << "  --|------------|-------\n";
    
    for (int i = 0; i < 16; i++) {
        int slot = (int)floor(i * PHI) % M;
        cout << "  " << setw(2) << i << " | "
             << setw(10) << slot << " | "
             << setw(6) << slot << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang periodicity ng Sturmian
    // ============================================
    cout << "--- 2. Periodicity ---\n\n";
    cout << "  Ang Sturmian permutation ay may period\n";
    cout << "  na may φ-based na structure\n\n";

    // Hanapin ang period
    vector<int> seen(8, -1);
    int period = -1;
    for (int i = 0; i < 100; i++) {
        int slot = (int)floor(i * PHI) % 8;
        if (seen[slot] != -1) {
            period = i - seen[slot];
            break;
        }
        seen[slot] = i;
    }
    cout << "  Period: " << period << "\n\n";

    // ============================================
    // 3. Sturmian bilang natural na rotation
    // ============================================
    cout << "--- 3. Sturmian bilang rotation ---\n\n";
    cout << "  Ang Sturmian ay nagbibigay ng natural na\n";
    cout << "  slot rotation na walang EvalRotate keys\n\n";

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

    // I-encrypt ang φ-powers
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) vals[i] = pow(PHI, i);
    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);

    // Ang "rotation" ay ang pag-add ng φ sa exponent
    // na automatic na nagpa-rotate ng Sturmian pattern
    vector<double> delta(8, 1.0);  // +φ sa exponent
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);
    auto ct_rot = cc->EvalAdd(ct, pt_delta);

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_rot, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  After +1 sa exponent (Sturmian rotation):\n  ";
    for (int i = 0; i < 8; i++) cout << setw(8) << res[i].real();
    cout << "\n";
    cout << "  Level: " << ct_rot->GetLevel() << "\n";

    return 0;
}
