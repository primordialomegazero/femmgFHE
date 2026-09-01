// ============================================
// φ-RULE 110 MULTIDIM NAND — MULTIDIMENSIONAL
//
// Slot 0: base log value (-2 o +2)
// Slots 1-7: φ-scaled versions
// Ang transition ay gumagamit ng multidimensional
// na φ-harmonic structure
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 MULTIDIM NAND\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;

    cout << "  ✅ CKKS initialized (depth 0, batch 8!)\n";
    cout << "  Multidim: slot 0 = base, slots 1-7 = φ-scaled\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_log = [&](int bit) {
        double base = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(8, 0.0);
        v[0] = base;
        for (int i = 1; i < 8; i++) {
            v[i] = base * pow(PHI, i);
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_base = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double base = result_pt->GetCKKSPackedValue()[0].real();
        return (base >= 0.0) ? 1 : 0;
    };

    // NAND na may multidimensional scaling
    auto nand_multi = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        auto negated = cc->EvalNegate(sum);
        // I-normalize: hatiin sa φ¹ para ma-preserve ang range
        return cc->EvalSub(negated, log(PHI));
    };

    auto not_gate = [&](const Ciphertext<DCRTPoly>& a) {
        return nand_multi(a, a);
    };

    auto and_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return not_gate(nand_multi(a, b));
    };

    auto or_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return nand_multi(not_gate(a), not_gate(b));
    };

    // Rule 110 na may tamang minterms
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        auto not_L = not_gate(L);
        auto not_C = not_gate(C);
        auto not_R = not_gate(R);
        
        // Output 1: 001, 010, 100, 101, 110
        auto t1 = and_gate(and_gate(not_L, not_C), R);   // 001
        auto t2 = and_gate(and_gate(not_L, C), not_R);   // 010
        auto t4 = and_gate(and_gate(L, not_C), not_R);   // 100
        auto t5 = and_gate(and_gate(L, not_C), R);       // 101
        auto t6 = and_gate(and_gate(L, C), not_R);       // 110
        
        auto or_1 = or_gate(t1, t2);
        auto or_2 = or_gate(or_1, t4);
        auto or_3 = or_gate(or_2, t5);
        return or_gate(or_3, t6);
    };

    // TRANSITION TABLE TEST
    cout << "========================================\n";
    cout << "  TRANSITION TABLE (MULTIDIM)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_log(L);
                auto ct_C = encrypt_log(C);
                auto ct_R = encrypt_log(R);
                
                auto ct_out = rule110_gate(ct_L, ct_C, ct_R);
                int output = decrypt_base(ct_out);
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
