// ΦΩ0 — BINFHE 32-BIT PARALLEL MULTIPLIER
// 12 threads, column-level parallelism
// Target: 32-bit in under 3 minutes
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>
#include <mutex>
#include <atomic>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

// Global state (thread-safe)
BinFHEContext g_cc;
LWEPrivateKey g_sk;
mutex g_mutex;
atomic<int> g_totalGates{0};
atomic<int> g_totalBootstraps{0};
atomic<long long> g_totalTimeMs{0};

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

// Thread-safe NAND with timing
LWECiphertext NAND_TS(const LWECiphertext& a, const LWECiphertext& b) {
    g_totalGates++;
    g_totalBootstraps++;
    
    auto start = high_resolution_clock::now();
    auto result = g_cc.Bootstrap(g_cc.EvalBinGate(NAND, a, b));
    auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    g_totalTimeMs += elapsed;
    
    return result;
}

LWECiphertext AND_TS(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_TS(a, b);
    return NAND_TS(n, clone_ct(n));
}

LWECiphertext XOR_TS(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_TS(a, b);
    return NAND_TS(NAND_TS(a, n), NAND_TS(b, n));
}

// ============================================
// PARALLEL PARTIAL PRODUCT GENERATION
// ============================================

void generate_partial_products(const vector<LWECiphertext>& a,
                                const vector<LWECiphertext>& b,
                                vector<vector<LWECiphertext>>& pp,
                                int start_row, int end_row,
                                int bits) {
    for(int i = start_row; i < end_row; i++) {
        for(int j = 0; j < bits; j++) {
            pp[i][j] = AND_TS(a[j], b[i]);
        }
    }
}

// ============================================
// OPTIMIZED 32-BIT MULTIPLIER (Parallel)
// ============================================

vector<LWECiphertext> multiply_32bit_parallel(const vector<LWECiphertext>& a,
                                                const vector<LWECiphertext>& b) {
    int bits = 32;
    auto zero = g_cc.Encrypt(g_sk, 0);
    
    // Initialize partial products matrix
    vector<vector<LWECiphertext>> pp(bits, vector<LWECiphertext>(bits));
    for(int i = 0; i < bits; i++)
        for(int j = 0; j < bits; j++)
            pp[i][j] = clone_ct(zero);
    
    // PHASE 1: Parallel partial product generation
    cout << "Φ Phase 1: Parallel partial products (12 threads)...\n";
    int numThreads = 12;
    int rowsPerThread = bits / numThreads;
    vector<thread> threads;
    
    auto phase1_start = high_resolution_clock::now();
    for(int t = 0; t < numThreads; t++) {
        int start_row = t * rowsPerThread;
        int end_row = (t == numThreads - 1) ? bits : start_row + rowsPerThread;
        threads.push_back(thread(generate_partial_products, 
                                 ref(a), ref(b), ref(pp),
                                 start_row, end_row, bits));
    }
    
    for(auto& t : threads) t.join();
    auto phase1_elapsed = duration_cast<seconds>(high_resolution_clock::now() - phase1_start).count();
    cout << "  ✅ Phase 1 complete: " << phase1_elapsed << "s | " 
         << g_totalGates << " gates\n";
    
    // PHASE 2: Column accumulation (parallel per column)
    cout << "Φ Phase 2: Parallel column accumulation...\n";
    vector<LWECiphertext> result(2*bits, clone_ct(zero));
    
    auto phase2_start = high_resolution_clock::now();
    
    // We'll do columns in parallel batches
    for(int col = 0; col < 2*bits; col++) {
        // Collect partial products in this column
        vector<LWECiphertext> terms;
        for(int i = 0; i < bits; i++) {
            int j = col - i;
            if(j >= 0 && j < bits) {
                terms.push_back(pp[i][j]);
            }
        }
        
        if(terms.empty()) {
            result[col] = clone_ct(zero);
        } else if(terms.size() == 1) {
            result[col] = clone_ct(terms[0]);
        } else {
            // XOR tree reduction
            auto sum = terms[0];
            for(size_t k = 1; k < terms.size(); k++) {
                sum = XOR_TS(sum, terms[k]);
            }
            result[col] = sum;
        }
    }
    
    auto phase2_elapsed = duration_cast<seconds>(high_resolution_clock::now() - phase2_start).count();
    cout << "  ✅ Phase 2 complete: " << phase2_elapsed << "s | " 
         << g_totalGates << " total gates\n";
    
    return result;
}

// ============================================
// DECRYPT RESULT
// ============================================

int64_t decrypt_result(const vector<LWECiphertext>& result, int bits) {
    int64_t val = 0;
    for(int i = 0; i < 2*bits; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1LL << i);
    }
    return val;
}

// ============================================
// MAIN
// ============================================

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE 32-BIT PARALLEL MULTIPLIER     ║\n";
    cout <<   "║  12 Threads | Target: <3 minutes             ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    cout << "Φ Initializing BinFHE (TOY parameters)...\n";
    g_cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    // Test case: 42 × 17 = 714
    int64_t testA = 42, testB = 17, expected = 714;
    
    cout << "Φ Test: " << testA << " × " << testB << " = " << expected << "\n";
    cout << "Φ Encrypting 32-bit inputs...\n";
    
    vector<LWECiphertext> a(32), b(32);
    for(int i = 0; i < 32; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    cout << "Φ Hardware threads: " << thread::hardware_concurrency() << "\n";
    cout << "Φ Starting parallel multiplication...\n\n";
    
    g_totalGates = 0;
    g_totalBootstraps = 0;
    g_totalTimeMs = 0;
    
    auto total_start = high_resolution_clock::now();
    auto result = multiply_32bit_parallel(a, b);
    auto total_elapsed = duration_cast<seconds>(high_resolution_clock::now() - total_start).count();
    
    cout << "\nΦ Decrypting result...\n";
    int64_t val = decrypt_result(result, 32);
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  32-BIT PARALLEL MULTIPLICATION RESULTS       ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  " << testA << " × " << testB << " = " << val << " (expected: " << expected << ")\n";
    cout <<   "║  Total gates: " << g_totalGates << "\n";
    cout <<   "║  Total bootstraps: " << g_totalBootstraps << "\n";
    cout <<   "║  Wall time: " << total_elapsed << " seconds\n";
    cout <<   "║  Avg gate time: " << (g_totalTimeMs / max((int)g_totalGates.load(), 1)) << "ms\n";
    cout <<   "║  Effective gates/sec: " << (g_totalGates / max((long long)total_elapsed, 1LL)) << "\n";
    cout <<   "║  Status: " << (val == expected ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
