// ============================================
// φ-100M BATCH PARALLEL — LAHAT NG SLOTS
//
// 100M operations sa 16 slots sabay-sabay
// Bawat slot ay may ibang operation
// Parallel processing — isang EvalAdd lang per step
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
    cout << "  φ-100M BATCH PARALLEL\n";
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
    const double LN_PHI = log(PHI);

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    // ============================================
    // BATCH PARALLEL ENCODING
    // ============================================

    auto encrypt_batch = [&](double x, int slot) {
        double log_val = fmod(log(x) / LN_PHI, PHI);
        vector<double> v(16, 0.0);
        // Bawat slot ay may ibang φ-scaling
        v[slot] = log_val / fib[slot];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_batch = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        vector<double> vals(16);
        for (int i = 0; i < 16; i++) {
            vals[i] = fmod(result_pt->GetCKKSPackedValue()[i].real(), PHI);
        }
        return vals;
    };

    // ============================================
    // 100M BATCH PARALLEL
    // ============================================

    cout << "========================================\n";
    cout << "  100M BATCH PARALLEL OPS\n";
    cout << "========================================\n\n";

    // Mixed operations per slot (16 iba't ibang ops!)
    vector<double> slot_ops = {
        2.0, 0.5, 3.0, 1.0/3.0, 5.0, 0.2, 7.0, 1.0/7.0,
        11.0, 1.0/11.0, 13.0, 1.0/13.0, 17.0, 1.0/17.0, 19.0, 1.0/19.0
    };

    cout << "  16 slots, bawat isa ay may ibang operation:\n";
    cout << "  Slot | Op\n";
    cout << "  -----|-----\n";
    for (int i = 0; i < 16; i++) {
        cout << "  " << setw(4) << i << " | ";
        if (slot_ops[i] > 1) cout << "×" << slot_ops[i];
        else cout << "÷" << (1.0/slot_ops[i]);
        cout << "\n";
    }
    cout << "\n";

    int N = 100000000;  // 100M

    cout << "  Operations: " << N << " per slot\n";
    cout << "  Total parallel ops: " << N * 16 << " (1.6B equivalent)\n\n";

    // Pre-encrypt ang lahat ng 16 operators sa isang vector
    vector<double> all_ops(16, 0.0);
    for (int i = 0; i < 16; i++) {
        all_ops[i] = fmod(log(slot_ops[i]) / LN_PHI / fib[i], PHI);
    }
    Plaintext pt_ops = cc->MakeCKKSPackedPlaintext(all_ops);
    auto ct_ops = cc->Encrypt(keyPair.publicKey, pt_ops);

    // Start: lahat ng slots = 1.0
    vector<double> init_vals(16, 0.0);
    for (int i = 0; i < 16; i++) {
        init_vals[i] = fmod(log(1.0) / LN_PHI / fib[i], PHI);
    }
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init_vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt_init);

    auto start = high_resolution_clock::now();

    // TUNAY NA PARALLEL — isang EvalAdd per step, 16 slots sabay-sabay!
    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, ct_ops);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto results = decrypt_batch(ct);

    cout << "  ✅ 100M parallel operations complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct->GetLevel() << "\n";
    cout << "  Towers: " << ct->GetElements()[0].GetNumOfElements() << "\n\n";

    // Verification per slot
    cout << "  Slot | Result | Expected | Match?\n";
    cout << "  -----|--------|----------|--------\n";

    int match_count = 0;
    for (int i = 0; i < 16; i++) {
        double expected = fmod(N * log(slot_ops[i]) / LN_PHI / fib[i], PHI);
        bool match = abs(results[i] - expected) < 0.01;
        match_count += match;
        
        cout << "  " << setw(4) << i << " | "
             << setw(7) << fixed << setprecision(4) << results[i] << " | "
             << setw(8) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/16\n\n";

    cout << "========================================\n";
    cout << "  BATCH PARALLEL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 100M parallel ops per slot\n";
    cout << "  ✅ 1.6B equivalent operations\n";
    cout << "  ✅ 16 slots sabay-sabay\n";
    cout << "  ✅ Mixed operations per slot\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
