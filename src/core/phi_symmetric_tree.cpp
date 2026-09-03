// ============================================
// φ-SYMMETRIC TREE — BALANCED ENCODING
//
// 0 → +φ/2
// 1 → -φ/2
// Tree + fold na may sign-aware decode
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
    cout << "  φ-SYMMETRIC TREE\n";
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
        // I-normalize sa [0, φ)
        double folded = fmod(val, PHI);
        if (folded < 0) folded += PHI;
        
        // Sa symmetric encoding:
        // even = malapit sa 0
        // odd = malapit sa ±φ/2
        if (folded < 0.2 || folded > PHI - 0.2) return 0;
        if (abs(folded - HALF_PHI) < 0.2 || abs(folded + HALF_PHI - PHI) < 0.2) return 1;
        
        // Fallback: gamitin ang sign
        return (val < 0) ? 1 : 0;
    };

    // ============================================
    // TREE REDUCTION (100 ONES)
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC TREE (100 ONES)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves;
    for (int i = 0; i < 100; i++) {
        leaves.push_back(encrypt_bit(1));
    }

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

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double avg = decrypt_avg(leaves[0]);
    int decoded = symmetric_decode(avg);
    int expected = 0;

    cout << "  Result: " << avg << "\n";
    cout << "  Decoded: " << decoded << " (Expected: " << expected << ")\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n\n";

    // ============================================
    // TREE REDUCTION (101 ONES)
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC TREE (101 ONES)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> leaves_odd;
    for (int i = 0; i < 101; i++) {
        leaves_odd.push_back(encrypt_bit(1));
    }

    while (leaves_odd.size() > 1) {
        vector<Ciphertext<DCRTPoly>> next_level;
        for (size_t i = 0; i + 1 < leaves_odd.size(); i += 2) {
            auto sum = cc->EvalAdd(leaves_odd[i], leaves_odd[i + 1]);
            next_level.push_back(sum);
        }
        if (leaves_odd.size() % 2 == 1) {
            next_level.push_back(leaves_odd.back());
        }
        leaves_odd = next_level;
    }

    double avg_odd = decrypt_avg(leaves_odd[0]);
    int decoded_odd = symmetric_decode(avg_odd);
    int expected_odd = 1;

    cout << "  Result: " << avg_odd << "\n";
    cout << "  Decoded: " << decoded_odd << " (Expected: " << expected_odd << ")\n";
    cout << "  Match: " << (decoded_odd == expected_odd ? "✅" : "❌") << "\n";
    cout << "  Level: " << leaves_odd[0]->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC TREE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Symmetric encoding tested\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
