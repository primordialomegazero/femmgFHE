// ΦΩ0 — BINFHE MAX PARALLEL (12 THREADS)
// Full CPU utilization for maximum gate throughput
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <algorithm>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext* g_cc = nullptr;
LWEPrivateKey g_sk = nullptr;

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

// Parallel NAND with configurable batch size
vector<LWECiphertext> parallel_nand_batch(
    const vector<pair<LWECiphertext, LWECiphertext>>& inputs,
    int num_threads = 12) {
    
    vector<future<vector<LWECiphertext>>> futures;
    int batch_size = (inputs.size() + num_threads - 1) / num_threads;
    
    for(int t = 0; t < num_threads; t++) {
        int start = t * batch_size;
        int end = min(start + batch_size, (int)inputs.size());
        
        if(start >= end) break;
        
        futures.push_back(async(launch::async, [start, end, &inputs]() {
            vector<LWECiphertext> batch_results;
            for(int i = start; i < end; i++) {
                auto result = g_cc->Bootstrap(
                    g_cc->EvalBinGate(NAND, inputs[i].first, inputs[i].second));
                batch_results.push_back(result);
            }
            return batch_results;
        }));
    }
    
    vector<LWECiphertext> results;
    for(auto& f : futures) {
        auto batch = f.get();
        results.insert(results.end(), batch.begin(), batch.end());
    }
    return results;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE MAX PARALLEL (12 THREADS)       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = cc.KeyGen();
    cc.BTKeyGen(g_sk);
    g_cc = &cc;

    int num_threads = thread::hardware_concurrency();
    cout << "Φ CPU Threads: " << num_threads << "\n\n";

    const int num_gates = 500;  // More gates for better measurement

    // Prepare inputs
    vector<pair<LWECiphertext, LWECiphertext>> inputs;
    for(int i = 0; i < num_gates; i++) {
        auto a = cc.Encrypt(g_sk, i % 2);
        auto b = cc.Encrypt(g_sk, (i+1) % 2);
        inputs.push_back({a, b});
    }

    cout << "=== SCALING TEST: " << num_gates << " NAND GATES ===\n\n";
    cout << "┌──────────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Threads      │ Time     │ Gates/s  │ Speedup  │\n";
    cout << "├──────────────┼──────────┼──────────┼──────────┤\n";

    double baseline_time = 0;

    vector<int> thread_counts = {1, 2, 4, 6, 8, 10, 12};

    for(int tc : thread_counts) {
        auto start = high_resolution_clock::now();
        
        auto results = parallel_nand_batch(inputs, tc);
        
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double secs = ms / 1000.0;
        double gates_per_sec = (num_gates / secs);
        
        if(tc == 1) baseline_time = secs;
        double speedup = baseline_time / secs;
        
        cout << "│ " << setw(4) << tc << " threads  │ ";
        cout << setw(6) << fixed << setprecision(1) << secs << "s   │ ";
        cout << setw(6) << fixed << setprecision(0) << gates_per_sec << "     │ ";
        cout << setw(6) << fixed << setprecision(1) << speedup << "×    │\n";
    }
    
    cout << "└──────────────┴──────────┴──────────┴──────────┘\n";

    // Project 32-bit multiplier time
    cout << "\n=== 32-BIT MULTIPLIER PROJECTION ===\n";
    cout << "  32-bit multiplier: ~31,529 gates\n\n";
    cout << "┌──────────────┬──────────────┐\n";
    cout << "│ Threads      │ Est. Time    │\n";
    cout << "├──────────────┼──────────────┤\n";
    
    double gates_per_sec_1 = 0;
    for(int tc : thread_counts) {
        auto start = high_resolution_clock::now();
        auto results = parallel_nand_batch(inputs, tc);
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double secs = ms / 1000.0;
        double gps = num_gates / secs;
        
        if(tc == 1) gates_per_sec_1 = gps;
        
        double est_32bit = 31529.0 / gps;
        
        cout << "│ " << setw(4) << tc << " threads  │ ";
        if(est_32bit >= 60.0) {
            cout << setw(6) << fixed << setprecision(1) << (est_32bit/60.0) << " min   │\n";
        } else {
            cout << setw(6) << fixed << setprecision(0) << est_32bit << " sec  │\n";
        }
    }
    cout << "└──────────────┴──────────────┘\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
