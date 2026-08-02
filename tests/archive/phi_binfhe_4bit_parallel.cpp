// ΦΩ0 — BINFHE 4-BIT PARALLEL
// Parallel gate execution for faster CT×CT
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <future>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext* g_cc = nullptr;
LWEPrivateKey g_sk = nullptr;

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

// Parallel NAND: compute 4 gates simultaneously
vector<LWECiphertext> parallel_nand_4(
    const vector<pair<LWECiphertext, LWECiphertext>>& inputs) {
    
    vector<future<LWECiphertext>> futures;
    
    for(auto& [a, b] : inputs) {
        futures.push_back(async(launch::async, [&]() {
            return g_cc->Bootstrap(g_cc->EvalBinGate(NAND, a, b));
        }));
    }
    
    vector<LWECiphertext> results;
    for(auto& f : futures) {
        results.push_back(f.get());
    }
    return results;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE 4-BIT PARALLEL                  ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = cc.KeyGen();
    cc.BTKeyGen(g_sk);
    g_cc = &cc;

    cout << "Φ Testing parallel vs sequential NAND gates...\n\n";

    // Test: 100 NAND gates
    const int num_gates = 100;
    
    // Prepare inputs
    vector<pair<LWECiphertext, LWECiphertext>> inputs;
    for(int i = 0; i < num_gates; i++) {
        auto a = cc.Encrypt(g_sk, i % 2);
        auto b = cc.Encrypt(g_sk, (i+1) % 2);
        inputs.push_back({a, b});
    }

    // Sequential
    auto start_seq = high_resolution_clock::now();
    for(auto& [a, b] : inputs) {
        auto result = cc.Bootstrap(cc.EvalBinGate(NAND, a, b));
    }
    auto end_seq = high_resolution_clock::now();
    auto seq_ms = duration_cast<milliseconds>(end_seq - start_seq).count();

    // Parallel (batches of 4)
    auto start_par = high_resolution_clock::now();
    for(int i = 0; i < num_gates; i += 4) {
        vector<pair<LWECiphertext, LWECiphertext>> batch;
        for(int j = i; j < min(i+4, num_gates); j++) {
            batch.push_back(inputs[j]);
        }
        auto results = parallel_nand_4(batch);
    }
    auto end_par = high_resolution_clock::now();
    auto par_ms = duration_cast<milliseconds>(end_par - start_par).count();

    cout << "  Sequential: " << seq_ms << "ms\n";
    cout << "  Parallel:   " << par_ms << "ms\n";
    double speedup = (double)seq_ms / par_ms;
    cout << "  Speedup:    " << fixed << setprecision(1) << speedup << "×\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
