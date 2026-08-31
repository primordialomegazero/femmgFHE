// ============================================
// φ-RULE 110 DEBUG — TAMANG TRANSITION
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
    cout << "  φ-RULE 110 DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    // ============================================
    // DEBUG: 3-BIT PATTERN → OUTPUT
    // ============================================

    cout << "  RULE 110 TABLE:\n";
    cout << "  L C R | Pattern | Expected Output\n";
    cout << "  ------|---------|----------------\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(7) << pattern << " | "
                     << setw(16) << rule110[pattern] << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // ENCODING TEST: ISANG 3-BIT PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  ENCODING TEST\n";
    cout << "========================================\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : PHI_INV;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_raw = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    };

    auto decode_bit = [&](const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        double normalized = (PHI - avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        return 1 - (int)round(mod2);
    };

    cout << "  Test: L=0, C=1, R=1 (pattern 011)\n";
    cout << "  Expected output: " << rule110[3] << " (011 → 1)\n\n";

    auto ct_L = encrypt_bit(0);
    auto ct_C = encrypt_bit(1);
    auto ct_R = encrypt_bit(1);

    // I-check ang individual bits
    cout << "  L decrypted: " << decode_bit(decrypt_raw(ct_L)) << " (expected 0)\n";
    cout << "  C decrypted: " << decode_bit(decrypt_raw(ct_C)) << " (expected 1)\n";
    cout << "  R decrypted: " << decode_bit(decrypt_raw(ct_R)) << " (expected 1)\n\n";

    // Transition: L + C + R
    auto sum1 = cc->EvalAdd(ct_L, ct_C);
    auto sum2 = cc->EvalAdd(sum1, ct_R);

    auto sum_vals = decrypt_raw(sum2);
    double avg_sum = 0.0;
    for (int i = 0; i < 16; i++) avg_sum += sum_vals[i].real();
    avg_sum /= 16.0;

    cout << "  L+C+R avg: " << avg_sum << "\n";
    cout << "  Decoded: " << decode_bit(sum_vals) << "\n";
    cout << "  Expected: 1 (Rule 110 output para sa 011)\n\n";

    // ============================================
    // ANG PROBLEMA
    // ============================================

    cout << "========================================\n";
    cout << "  ANG PROBLEMA\n";
    cout << "========================================\n\n";

    cout << "  Ang L+C+R ay hindi Rule 110 lookup!\n";
    cout << "  Kailangan natin ng tamang transition:\n\n";

    cout << "  Rule 110 formula:\n";
    cout << "  next = NOT(L) AND C AND R  OR  L AND NOT(C) AND R  OR...\n\n";

    cout << "  O mas simple:\n";
    cout << "  next = 1 kung (L,C,R) ay isa sa:\n";
    cout << "  001, 010, 011, 101, 110\n\n";

    cout << "  Kailangan natin ng LOOKUP — hindi sum!\n\n";

    return 0;
}
