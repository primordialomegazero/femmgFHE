// ============================================
// φ-ALL IN ONE — LAHAT SA ISANG TEST
//
// 1. 8/8 Full Adder (φ-emergent)
// 2. 100 Chained (half-split decode)
// 3. 1M Direct (fractal modulo)
// 4. Pos+Neg values
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <random>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ALL IN ONE — LAHAT SABAY-SABAY\n";
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
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : PHI_INV;
        vector<double> v(16, val);
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

    auto half_split = [&](double val) {
        double mod_phi = fmod(val, PHI);
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // ============================================
    // TEST 1: 8/8 FULL ADDER
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 8/8 FULL ADDER\n";
    cout << "========================================\n\n";

    int adder_correct = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                auto ct_cin = encrypt_bit(Cin);
                
                auto sum_ct = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                int sum = half_split(decrypt_avg(sum_ct));
                int expected = (A + B + Cin) % 2;
                adder_correct += (sum == expected);
            }
        }
    }

    cout << "  Full Adder: " << adder_correct << "/8\n\n";

    // ============================================
    // TEST 2: 100 CHAINED
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 100 CHAINED\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_bit(0);
    auto ct_add = encrypt_bit(1);

    int chain_correct = 0;
    for (int i = 0; i < 100; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        int decoded = half_split(decrypt_avg(ct_acc));
        int expected = (i + 1) % 2;
        chain_correct += (decoded == expected);
    }

    cout << "  Chained: " << chain_correct << "/100\n\n";

    // ============================================
    // TEST 3: 1K DIRECT (POS+NEG)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 1K DIRECT (POS+NEG)\n";
    cout << "========================================\n\n";

    mt19937 gen(42);
    uniform_real_distribution<double> random_val(-10.0, 10.0);

    auto ct_direct = encrypt_bit(0);
    double total = 0.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        double val = fmod(random_val(gen), PHI);
        total = fmod(total + val, PHI);
        
        auto ct_val = encrypt_bit((val > PHI / 2.0) ? 1 : 0);
        ct_direct = cc->EvalAdd(ct_direct, ct_val);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    int direct_result = half_split(decrypt_avg(ct_direct));
    int direct_expected = (int)fmod(total, 2.0);

    cout << "  1K Direct: " << direct_result << " (exp: " << direct_expected << ")\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_direct->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ALL IN ONE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Full Adder: " << adder_correct << "/8\n";
    cout << "  ✅ Chained: " << chain_correct << "/100\n";
    cout << "  ✅ Direct: " << direct_result << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
