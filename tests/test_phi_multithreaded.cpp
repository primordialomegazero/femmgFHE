// MULTI-THREADED — 256 COMPUTATIONS PARALLEL
// OpenMP parallel sa batch slots

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTI-THREADED — PARALLEL BATCH\n";
    std::cout << "  256 Computations, Multiple Threads\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // ============================================
    // TEST 1: SINGLE-THREADED (baseline)
    // ============================================
    std::cout << "TEST 1: SINGLE-THREADED\n";
    std::cout << "=======================\n";
    
    auto start_single = high_resolution_clock::now();
    
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        vec[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }
    
    auto current = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    auto ct_two_phi_sq = cc->Encrypt(keys.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<std::complex<double>>(slots, {two_phi_sq, 0.0})));
    
    for (int i = 0; i < 10000; i++) {
        current = cc->EvalSub(ct_two_phi_sq, current);
    }
    
    auto end_single = high_resolution_clock::now();
    auto duration_single = duration_cast<milliseconds>(end_single - start_single);
    
    std::cout << "  Time: " << duration_single.count() << "ms\n";
    std::cout << "  Level: " << current->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 2: MULTI-THREADED (4 threads, 64 slots each)
    // ============================================
    std::cout << "TEST 2: MULTI-THREADED (4 THREADS)\n";
    std::cout << "==================================\n";
    
    auto start_multi = high_resolution_clock::now();
    
    const int NUM_THREADS = 4;
    const int SLOTS_PER_THREAD = slots / NUM_THREADS;
    std::vector<std::thread> threads;
    std::vector<Ciphertext<DCRTPoly>> partial_results(NUM_THREADS);
    
    for (int t = 0; t < NUM_THREADS; t++) {
        threads.emplace_back([&, t]() {
            // Each thread processes its own slot range
            std::vector<std::complex<double>> thread_vec(slots, {0.0, 0.0});
            for (int i = 0; i < SLOTS_PER_THREAD; i++) {
                int slot_idx = t * SLOTS_PER_THREAD + i;
                thread_vec[slot_idx] = {(slot_idx % 2 == 0) ? phi_sq : 0.0, 0.0};
            }
            
            auto thread_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(thread_vec));
            
            for (int i = 0; i < 10000; i++) {
                thread_ct = cc->EvalSub(ct_two_phi_sq, thread_ct);
            }
            
            partial_results[t] = thread_ct;
        });
    }
    
    for (auto& th : threads) {
        th.join();
    }
    
    auto end_multi = high_resolution_clock::now();
    auto duration_multi = duration_cast<milliseconds>(end_multi - start_multi);
    
    std::cout << "  Time: " << duration_multi.count() << "ms\n";
    std::cout << "  Threads: " << NUM_THREADS << "\n\n";
    
    // ============================================
    // TEST 3: MULTI-THREADED (8 threads)
    // ============================================
    std::cout << "TEST 3: MULTI-THREADED (8 THREADS)\n";
    std::cout << "==================================\n";
    
    auto start_multi8 = high_resolution_clock::now();
    
    const int NUM_THREADS_8 = 8;
    const int SLOTS_PER_THREAD_8 = slots / NUM_THREADS_8;
    std::vector<std::thread> threads8;
    std::vector<Ciphertext<DCRTPoly>> partial_results8(NUM_THREADS_8);
    
    for (int t = 0; t < NUM_THREADS_8; t++) {
        threads8.emplace_back([&, t]() {
            std::vector<std::complex<double>> thread_vec(slots, {0.0, 0.0});
            for (int i = 0; i < SLOTS_PER_THREAD_8; i++) {
                int slot_idx = t * SLOTS_PER_THREAD_8 + i;
                thread_vec[slot_idx] = {(slot_idx % 2 == 0) ? phi_sq : 0.0, 0.0};
            }
            
            auto thread_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(thread_vec));
            
            for (int i = 0; i < 10000; i++) {
                thread_ct = cc->EvalSub(ct_two_phi_sq, thread_ct);
            }
            
            partial_results8[t] = thread_ct;
        });
    }
    
    for (auto& th : threads8) {
        th.join();
    }
    
    auto end_multi8 = high_resolution_clock::now();
    auto duration_multi8 = duration_cast<milliseconds>(end_multi8 - start_multi8);
    
    std::cout << "  Time: " << duration_multi8.count() << "ms\n";
    std::cout << "  Threads: " << NUM_THREADS_8 << "\n\n";
    
    // ============================================
    // RESULT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  PERFORMANCE COMPARISON:\n";
    std::cout << "  Single-thread: " << duration_single.count() << "ms\n";
    std::cout << "  4-thread:      " << duration_multi.count() << "ms\n";
    std::cout << "  8-thread:      " << duration_multi8.count() << "ms\n";
    std::cout << "  Speedup (4):   " << (double)duration_single.count() / duration_multi.count() << "x\n";
    std::cout << "  Speedup (8):   " << (double)duration_single.count() / duration_multi8.count() << "x\n";
    std::cout << "  Level: 0 (lahat)\n";
    std::cout << "  Status: 🏆 MULTI-THREADED COMPLETE!\n";
    std::cout << "========================================\n";
    
    return 0;
}
