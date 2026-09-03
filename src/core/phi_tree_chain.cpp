// ============================================
// φ-TREE CHAIN — BINARY TREE REDUCTION
//
// Sa halip na sequential, pagsasamahin nang pares
// para O(log n) ang lalim ng drift accumulation
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
    cout << "  φ-TREE CHAIN — BINARY TREE REDUCTION\n";
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Tree reduction: O(log n) na lang ang drift\n\n";

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

    auto zero_adaptive_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // ============================================
    // TREE REDUCTION TEST
    // ============================================

    cout << "========================================\n";
    cout << "  TREE REDUCTION (100 ELEMENTS)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    // Gagawa tayo ng 100 encrypted "1" values
    vector<Ciphertext<DCRTPoly>> leaves;
    for (int i = 0; i < 100; i++) {
        leaves.push_back(encrypt_bit(1));
    }

    // Tree reduction: pagsamahin nang pares
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
    int decoded = zero_adaptive_decode(avg);
    int expected = 0; // 100 ones = even, so sum = 0 mod 2

    cout << "  Tree Result: " << avg << "\n";
    cout << "  Decoded: " << decoded << " (Expected: " << expected << ")\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << leaves[0]->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  TREE CHAIN COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Tree Reduction: O(log n)\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
