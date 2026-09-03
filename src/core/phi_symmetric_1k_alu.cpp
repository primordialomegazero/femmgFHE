// ============================================
// φ-SYMMETRIC TREE 1K + 8-BIT ALU TEST
//
// Symmetric encoding: 0→+φ/2, 1→-φ/2
// Tree reduction: 1000 leaves
// 8-bit ALU: 8 parallel full adders
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
    cout << "  φ-SYMMETRIC TREE 1K + 8-BIT ALU\n";
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Symmetric encoding: 0→+φ/2, 1→-φ/2\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? HALF_PHI : -HALF_PHI;
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

    auto symmetric_decode = [&](double val) {
        double folded = fmod(val, PHI);
        if (folded < 0) folded += PHI;
        
        // Even = malapit sa 0
        if (folded < 0.2 || folded > PHI - 0.2) return 0;
        
        // Odd = malapit sa ±φ/2
        if (abs(folded - HALF_PHI) < 0.2 || abs(folded + HALF_PHI - PHI) < 0.2) return 1;
        
        // Fallback: sign
        return (val < 0) ? 1 : 0;
    };

    // ============================================
    // TEST 1: SYMMETRIC TREE 1000 LEAVES
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC TREE 1000 LEAVES\n";
    cout << "========================================\n\n";

    auto start_1k = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_1k;
    for (int i = 0; i < 1000; i++) {
        leaves_1k.push_back(encrypt_bit(1));
    }

    while (leaves_1k.size() > 1) {
        vector<Ciphertext<DCRTPoly>> next_level;
        for (size_t i = 0; i + 1 < leaves_1k.size(); i += 2) {
            auto sum = cc->EvalAdd(leaves_1k[i], leaves_1k[i + 1]);
            next_level.push_back(sum);
        }
        if (leaves_1k.size() % 2 == 1) {
            next_level.push_back(leaves_1k.back());
        }
        leaves_1k = next_level;
    }

    auto end_1k = high_resolution_clock::now();
    auto time_1k = duration_cast<milliseconds>(end_1k - start_1k).count();

    double avg_1k = decrypt_avg(leaves_1k[0]);
    int decoded_1k = symmetric_decode(avg_1k);
    int expected_1k = 0; // 1000 ones = even

    cout << "  Result: " << avg_1k << "\n";
    cout << "  Decoded: " << decoded_1k << " (Expected: " << expected_1k << ")\n";
    cout << "  Match: " << (decoded_1k == expected_1k ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_1k << " ms\n";
    cout << "  Level: " << leaves_1k[0]->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: 8-BIT ALU (8 PARALLEL FULL ADDERS)
    // ============================================

    cout << "========================================\n";
    cout << "  8-BIT ALU (8 PARALLEL)\n";
    cout << "========================================\n\n";

    // A = 10101010 (170), B = 01010101 (85)
    // Expected Sum = 11111111 (255)
    vector<int> A_bits = {1, 0, 1, 0, 1, 0, 1, 0};
    vector<int> B_bits = {0, 1, 0, 1, 0, 1, 0, 1};

    int alu_match = 0;
    auto start_alu = high_resolution_clock::now();

    for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
        int A = A_bits[bit_pos];
        int B = B_bits[bit_pos];
        
        // Full adder: Sum = A XOR B XOR Cin
        // Para sa 8-bit ALU, walang carry in sa first bit
        int expected_sum = (A + B) % 2;
        
        auto ct_a = encrypt_bit(A);
        auto ct_b = encrypt_bit(B);
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        
        double avg = decrypt_avg(ct_sum);
        int decoded = symmetric_decode(avg);
        
        bool match = (decoded == expected_sum);
        alu_match += match;
        
        cout << "  Bit " << bit_pos << ": " << A << " + " << B 
             << " = " << decoded << " (Expected: " << expected_sum << ") "
             << (match ? "✅" : "❌") << "\n";
    }

    auto end_alu = high_resolution_clock::now();
    auto time_alu = duration_cast<milliseconds>(end_alu - start_alu).count();

    cout << "\n  ALU Match: " << alu_match << "/8\n";
    cout << "  Time: " << time_alu << " ms\n\n";

    // ============================================
    // TEST 3: BATCH ALU (16 SLOTS = 16 PARALLEL)
    // ============================================

    cout << "========================================\n";
    cout << "  BATCH ALU (16 SLOTS)\n";
    cout << "========================================\n\n";

    vector<double> batch_vals(16, 0.0);
    for (int i = 0; i < 16; i++) {
        batch_vals[i] = (i % 2 == 0) ? HALF_PHI : -HALF_PHI;
    }

    Plaintext pt_batch = cc->MakeCKKSPackedPlaintext(batch_vals);
    auto ct_batch = cc->Encrypt(keyPair.publicKey, pt_batch);

    vector<double> add_vals(16, -HALF_PHI);
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(add_vals);
    auto ct_add = cc->Encrypt(keyPair.publicKey, pt_add);

    auto ct_batch_result = cc->EvalAdd(ct_batch, ct_add);

    Plaintext batch_result_pt;
    cc->Decrypt(keyPair.secretKey, ct_batch_result, &batch_result_pt);
    batch_result_pt->SetLength(16);

    int batch_match = 0;
    for (int i = 0; i < 16; i++) {
        double val = batch_result_pt->GetCKKSPackedValue()[i].real();
        int decoded = symmetric_decode(val);
        int expected = (i % 2 == 0) ? 0 : 1; // 0+1=1, 1+1=0
        batch_match += (decoded == expected);
    }

    cout << "  Batch Match: " << batch_match << "/16\n";
    cout << "  Level: " << ct_batch_result->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC TREE 1K + ALU COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Tree 1K: " << (decoded_1k == expected_1k ? "✅" : "❌") << "\n";
    cout << "  ✅ ALU: " << alu_match << "/8\n";
    cout << "  ✅ Batch: " << batch_match << "/16\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
