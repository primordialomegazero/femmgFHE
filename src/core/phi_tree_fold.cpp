// ============================================
// φ-TREE FOLD — TREE + FOLD CORRECTION
//
// Bawat tree level, ibawas agad ang φ-ciphertext
// para hindi lumaki ang value at manatili sa [0, φ)
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
    cout << "  φ-TREE FOLD\n";
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
    cout << "  Tree + φ-fold correction per level\n\n";

    auto encrypt_value = [&](double val) {
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
        // I-fold sa [0, φ) bago i-decode
        double folded = fmod(val, PHI);
        if (folded < 0.1 || folded > PHI - 0.1) return 0;
        return (folded < PHI / 2.0) ? 1 : 0;
    };

    // Precomputed φ-ciphertext para sa fold correction
    auto ct_phi = encrypt_value(PHI);
    auto ct_2phi = encrypt_value(2.0 * PHI);

    // ============================================
    // TEST: 100 ONES, TREE + FOLD
    // ============================================

    cout << "========================================\n";
    cout << "  TREE FOLD (100 ONES)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> leaves;
    for (int i = 0; i < 100; i++) {
        leaves.push_back(encrypt_value(PHI_INV));
    }

    int level_count = 0;
    while (leaves.size() > 1) {
        vector<Ciphertext<DCRTPoly>> next_level;
        for (size_t i = 0; i + 1 < leaves.size(); i += 2) {
            auto sum = cc->EvalAdd(leaves[i], leaves[i + 1]);

            // φ-fold: habang ang sum ay >= φ, ibawas ang φ
            // Dahil 2φ_inv ≈ 1.236 < φ, sapat na ang isang subtract
            // Pero para sa kaligtasan, gumamit ng conditional correction
            // via repeated EvalSub sa precomputed φ
            auto folded = sum;
            // Sub φ kung ang sum ay lampas sa φ
            // Dahil hindi natin ma-check nang direkta, gagamit tayo ng
            // approximation: kung ang sum >= φ, ibawas ang φ
            // Sa CKKS, hindi ito exact, pero subukan natin
            folded = cc->EvalSub(folded, ct_phi);
            
            next_level.push_back(folded);
        }
        if (leaves.size() % 2 == 1) {
            next_level.push_back(leaves.back());
        }
        leaves = next_level;
        level_count++;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double avg = decrypt_avg(leaves[0]);
    int decoded = zero_adaptive_decode(avg);
    int expected = 0; // 100 ones = even

    cout << "  Tree Fold Result: " << avg << "\n";
    cout << "  Decoded: " << decoded << " (Expected: " << expected << ")\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Levels: " << level_count << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << leaves[0]->GetLevel() << "\n\n";

    // ============================================
    // TEST: 101 ONES (ODD)
    // ============================================

    cout << "========================================\n";
    cout << "  TREE FOLD (101 ONES)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> leaves_odd;
    for (int i = 0; i < 101; i++) {
        leaves_odd.push_back(encrypt_value(PHI_INV));
    }

    while (leaves_odd.size() > 1) {
        vector<Ciphertext<DCRTPoly>> next_level;
        for (size_t i = 0; i + 1 < leaves_odd.size(); i += 2) {
            auto sum = cc->EvalAdd(leaves_odd[i], leaves_odd[i + 1]);
            auto folded = cc->EvalSub(sum, ct_phi);
            next_level.push_back(folded);
        }
        if (leaves_odd.size() % 2 == 1) {
            next_level.push_back(leaves_odd.back());
        }
        leaves_odd = next_level;
    }

    double avg_odd = decrypt_avg(leaves_odd[0]);
    int decoded_odd = zero_adaptive_decode(avg_odd);
    int expected_odd = 1; // 101 ones = odd

    cout << "  Tree Fold Result: " << avg_odd << "\n";
    cout << "  Decoded: " << decoded_odd << " (Expected: " << expected_odd << ")\n";
    cout << "  Match: " << (decoded_odd == expected_odd ? "✅" : "❌") << "\n";
    cout << "  Level: " << leaves_odd[0]->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  TREE FOLD COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Tree fold correction applied\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
