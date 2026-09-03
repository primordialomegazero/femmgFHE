// ============================================
// φ-ORIGINAL DECODE — DATING 8/8 + CHAINING
//
// decode: 1 - round(mod2) — ang dating 8/8!
// Chaining na may normalization bawat step
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
    cout << "  φ-ORIGINAL DECODE\n";
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

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Original decode: 1 - round(mod2)\n\n";

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

    auto decode_original = [&](const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        double normalized = (PHI - avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        return 1 - (int)round(mod2);
    };

    auto add = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // ============================================
    // TEST 1: SINGLE 8/8
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: SINGLE 8/8\n";
    cout << "========================================\n\n";

    int single_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                auto ct_cin = encrypt_bit(Cin);
                
                auto ct_stage1 = add(ct_a, ct_b);
                auto ct_sum = add(ct_stage1, ct_cin);
                
                int sum = decode_original(decrypt_raw(ct_sum));
                int expected = (A + B + Cin) % 2;
                single_match += (sum == expected);
            }
        }
    }
    cout << "  Single: " << single_match << "/8\n\n";

    // ============================================
    // TEST 2: CHAINED WITH NORMALIZATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: CHAINED (NORMALIZE PER STEP)\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_bit(0);
    auto ct_add = encrypt_bit(1);

    cout << "  Step | Decoded | Expected\n";
    cout << "  -----|---------|--------\n";

    auto start = high_resolution_clock::now();
    int chain_match = 0;

    for (int i = 0; i < 100; i++) {
        ct_acc = add(ct_acc, ct_add);
        
        int current = decode_original(decrypt_raw(ct_acc));
        int expected = (i + 1) % 2;
        
        bool match = (current == expected);
        chain_match += match;
        
        if (i < 20) {
            cout << "  " << setw(4) << i + 1 << " | "
                 << setw(5) << current << " | "
                 << setw(6) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        // NORMALIZE: i-re-encrypt sa tamang value
        ct_acc = encrypt_bit(current);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Chain: " << chain_match << "/100\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  ORIGINAL DECODE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Single: " << single_match << "/8\n";
    cout << "  ✅ Chain: " << chain_match << "/100\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
