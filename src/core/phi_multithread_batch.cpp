// ============================================
// φ-MULTITHREAD + BATCH — Optimized Parallel
// Batch processing + OpenMP multithreading
// Batch size 32 para sa mas maraming chains
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <omp.h>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(32);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double n) {
        vector<double> v(32, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(32);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 32; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-MULTITHREAD + BATCH — Optimized\n";
    cout << "========================================\n\n";
    cout << "  Batch size 32 + OpenMP\n\n";

    // ============================================
    // TEST 1: Batch processing — 32 slots
    // ============================================
    cout << "  TEST 1: Batch processing — 32 slots\n\n";

    // Pre-encode unique deltas
    vector<double> pattern = {1.0, -0.5, 2.0, -1.5, 0.5, -2.0, 1.5, -0.5};
    vector<Plaintext> unique_encoded;
    
    for (double delta : pattern) {
        vector<double> d(32, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        unique_encoded.push_back(pt);
    }

    auto ct = encrypt_state(10.0);
    
    int N = 100000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, unique_encoded[i % unique_encoded.size()]);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final: " << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Effective ops/sec (32 slots): " << (N * 32 * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Multithreaded — 8 threads
    // ============================================
    cout << "  TEST 2: Multithreaded — 8 threads\n\n";

    int num_threads = 8;
    cout << "    Using " << num_threads << " threads\n\n";

    // Bawat thread ay may sariling state at delta
    vector<Ciphertext<DCRTPoly>> thread_states(num_threads);
    for (int t = 0; t < num_threads; t++) {
        thread_states[t] = encrypt_state(10.0 + t);
    }

    start = high_resolution_clock::now();
    
    #pragma omp parallel for num_threads(num_threads)
    for (int t = 0; t < num_threads; t++) {
        for (int i = 0; i < N / num_threads; i++) {
            thread_states[t] = cc->EvalAdd(thread_states[t], unique_encoded[i % unique_encoded.size()]);
        }
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Total ops: " << N << "\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Effective ops/sec (8 threads × 32 slots): " << (N * 32 * 1000.0) / time << "\n\n";

    // ============================================
    // TEST 3: Combined batch + multithread
    // ============================================
    cout << "  TEST 3: Combined batch + multithread\n\n";

    // Pre-encode na batch ng 8 deltas sa isang plaintext
    vector<double> batch_delta(32, 0.0);
    for (int i = 0; i < 32; i++) {
        batch_delta[i] = pattern[i % pattern.size()];
    }
    Plaintext pt_batch = cc->MakeCKKSPackedPlaintext(batch_delta);

    ct = encrypt_state(10.0);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, pt_batch);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    v_final = decrypt_state(ct);
    
    cout << "    Final: " << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Effective ops/sec (32 unique chains): " << (N * 32 * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
