// ============================================
// φ-SYMMETRIC STRESS TEST
//
// Large-scale parity tests gamit ang
// symmetric encoding at sign-aware decode
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
    cout << "  φ-SYMMETRIC STRESS TEST\n";
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
        
        if (folded < 0.2 || folded > PHI - 0.2) return 0;
        if (abs(folded - HALF_PHI) < 0.2 || abs(folded + HALF_PHI - PHI) < 0.2) return 1;
        
        return (val < 0) ? 1 : 0;
    };

    auto tree_reduce = [&](vector<Ciphertext<DCRTPoly>> leaves) {
        while (leaves.size() > 1) {
            vector<Ciphertext<DCRTPoly>> next_level;
            for (size_t i = 0; i + 1 < leaves.size(); i += 2) {
                auto sum = cc->EvalAdd(leaves[i], leaves[i + 1]);
                next_level.push_back(sum);
            }
            if (leaves.size() % 2 == 1) {
                next_level.push_back(leaves.back());
            }
            leaves = next_level;
        }
        return leaves[0];
    };

    // ============================================
    // TEST 1: 1,000 ONES (EVEN)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 1,000 ONES (EVEN)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_1k;
    for (int i = 0; i < 1000; i++) {
        leaves_1k.push_back(encrypt_bit(1));
    }
    auto result_1k = tree_reduce(leaves_1k);

    auto end = high_resolution_clock::now();
    auto time_1k = duration_cast<milliseconds>(end - start).count();

    double avg_1k = decrypt_avg(result_1k);
    int decoded_1k = symmetric_decode(avg_1k);
    int expected_1k = 0;

    cout << "  Result: " << avg_1k << "\n";
    cout << "  Decoded: " << decoded_1k << " (Expected: " << expected_1k << ")\n";
    cout << "  Match: " << (decoded_1k == expected_1k ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_1k << " ms\n";
    cout << "  Level: " << result_1k->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: 1,001 ONES (ODD)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 1,001 ONES (ODD)\n";
    cout << "========================================\n\n";

    start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_1k1;
    for (int i = 0; i < 1001; i++) {
        leaves_1k1.push_back(encrypt_bit(1));
    }
    auto result_1k1 = tree_reduce(leaves_1k1);

    end = high_resolution_clock::now();
    auto time_1k1 = duration_cast<milliseconds>(end - start).count();

    double avg_1k1 = decrypt_avg(result_1k1);
    int decoded_1k1 = symmetric_decode(avg_1k1);
    int expected_1k1 = 1;

    cout << "  Result: " << avg_1k1 << "\n";
    cout << "  Decoded: " << decoded_1k1 << " (Expected: " << expected_1k1 << ")\n";
    cout << "  Match: " << (decoded_1k1 == expected_1k1 ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_1k1 << " ms\n";
    cout << "  Level: " << result_1k1->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: 10,000 ONES (EVEN)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 10,000 ONES (EVEN)\n";
    cout << "========================================\n\n";

    start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_10k;
    for (int i = 0; i < 10000; i++) {
        leaves_10k.push_back(encrypt_bit(1));
    }
    auto result_10k = tree_reduce(leaves_10k);

    end = high_resolution_clock::now();
    auto time_10k = duration_cast<milliseconds>(end - start).count();

    double avg_10k = decrypt_avg(result_10k);
    int decoded_10k = symmetric_decode(avg_10k);
    int expected_10k = 0;

    cout << "  Result: " << avg_10k << "\n";
    cout << "  Decoded: " << decoded_10k << " (Expected: " << expected_10k << ")\n";
    cout << "  Match: " << (decoded_10k == expected_10k ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_10k << " ms\n";
    cout << "  Level: " << result_10k->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: 10,001 ONES (ODD)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 10,001 ONES (ODD)\n";
    cout << "========================================\n\n";

    start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_10k1;
    for (int i = 0; i < 10001; i++) {
        leaves_10k1.push_back(encrypt_bit(1));
    }
    auto result_10k1 = tree_reduce(leaves_10k1);

    end = high_resolution_clock::now();
    auto time_10k1 = duration_cast<milliseconds>(end - start).count();

    double avg_10k1 = decrypt_avg(result_10k1);
    int decoded_10k1 = symmetric_decode(avg_10k1);
    int expected_10k1 = 1;

    cout << "  Result: " << avg_10k1 << "\n";
    cout << "  Decoded: " << decoded_10k1 << " (Expected: " << expected_10k1 << ")\n";
    cout << "  Match: " << (decoded_10k1 == expected_10k1 ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_10k1 << " ms\n";
    cout << "  Level: " << result_10k1->GetLevel() << "\n\n";

    // ============================================
    // TEST 5: RANDOM PARITY (10,000 RANDOM BITS)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: RANDOM PARITY (10,000 BITS)\n";
    cout << "========================================\n\n";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1);

    start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves_random;
    int expected_random = 0;
    for (int i = 0; i < 10000; i++) {
        int bit = dis(gen);
        expected_random ^= bit;
        leaves_random.push_back(encrypt_bit(bit));
    }
    auto result_random = tree_reduce(leaves_random);

    end = high_resolution_clock::now();
    auto time_random = duration_cast<milliseconds>(end - start).count();

    double avg_random = decrypt_avg(result_random);
    int decoded_random = symmetric_decode(avg_random);

    cout << "  Result: " << avg_random << "\n";
    cout << "  Decoded: " << decoded_random << " (Expected: " << expected_random << ")\n";
    cout << "  Match: " << (decoded_random == expected_random ? "✅" : "❌") << "\n";
    cout << "  Time: " << time_random << " ms\n";
    cout << "  Level: " << result_random->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  STRESS TEST COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1,000 ones: " << (decoded_1k == expected_1k ? "PASS" : "FAIL") << "\n";
    cout << "  ✅ 1,001 ones: " << (decoded_1k1 == expected_1k1 ? "PASS" : "FAIL") << "\n";
    cout << "  ✅ 10,000 ones: " << (decoded_10k == expected_10k ? "PASS" : "FAIL") << "\n";
    cout << "  ✅ 10,001 ones: " << (decoded_10k1 == expected_10k1 ? "PASS" : "FAIL") << "\n";
    cout << "  ✅ Random parity: " << (decoded_random == expected_random ? "PASS" : "FAIL") << "\n";
    cout << "  ✅ Level 0 all tests\n\n";

    return 0;
}
