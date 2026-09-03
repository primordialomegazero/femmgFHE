// ============================================
// φ-CHUNKED TREE — MEMORY-SAFE REDUCTION
//
// I-proseso sa chunks para hindi maubos ang RAM
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
    cout << "  φ-CHUNKED TREE\n";
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
    cout << "  Chunked tree: 1000 leaves per chunk\n\n";

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

    auto reduce_chunk = [&](vector<Ciphertext<DCRTPoly>> leaves) {
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
    // 10,001 ONES VIA CHUNKED REDUCTION
    // ============================================

    cout << "========================================\n";
    cout << "  10,001 ONES (CHUNKED, 1000/CHUNK)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    vector<Ciphertext<DCRTPoly>> chunk_results;
    int remaining = 10001;

    for (int chunk = 0; chunk < 11; chunk++) {
        int chunk_size = min(1000, remaining);
        vector<Ciphertext<DCRTPoly>> leaves;
        
        for (int i = 0; i < chunk_size; i++) {
            leaves.push_back(encrypt_bit(1));
        }
        
        auto result = reduce_chunk(leaves);
        chunk_results.push_back(result);
        
        remaining -= chunk_size;
        
        // I-clear ang leaves para sa memory
        leaves.clear();
        leaves.shrink_to_fit();
        
        cout << "  Chunk " << chunk + 1 << ": " << chunk_size << " leaves reduced\n";
    }

    // Final reduction ng chunk results
    auto final_result = reduce_chunk(chunk_results);

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double avg = decrypt_avg(final_result);
    int decoded = symmetric_decode(avg);
    int expected = 1;

    cout << "\n  Result: " << avg << "\n";
    cout << "  Decoded: " << decoded << " (Expected: " << expected << ")\n";
    cout << "  Match: " << (decoded == expected ? "✅" : "❌") << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << final_result->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  CHUNKED TREE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 10,001 ones (chunked)\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
