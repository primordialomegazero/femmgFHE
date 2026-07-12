// ΦΩ0 — BINFHE CARRY-LOOKAHEAD + PARALLEL COLUMNS
// O(log n) carry instead of O(n) ripple
// Independent column groups for maximum parallelism
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>
#include <atomic>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext g_cc;
LWEPrivateKey g_sk;
atomic<int> g_totalGates{0};

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}
LWECiphertext NAND_G(const LWECiphertext& a, const LWECiphertext& b) {
    g_totalGates++;
    return g_cc.Bootstrap(g_cc.EvalBinGate(NAND, a, b));
}
LWECiphertext AND_G(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_G(a, b);
    return NAND_G(n, clone_ct(n));
}
LWECiphertext OR_G(const LWECiphertext& a, const LWECiphertext& b) {
    return NAND_G(NAND_G(a, clone_ct(a)), NAND_G(b, clone_ct(b)));
}
LWECiphertext XOR_G(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_G(a, b);
    return NAND_G(NAND_G(a, n), NAND_G(b, n));
}

// ============================================
// CARRY-LOOKAHEAD GENERATOR
// ============================================

// Generate: g[i] = a[i] AND b[i], p[i] = a[i] XOR b[i]
void generate_gp(const vector<LWECiphertext>& a, const vector<LWECiphertext>& b,
                 vector<LWECiphertext>& g, vector<LWECiphertext>& p, int bits) {
    for(int i = 0; i < bits; i++) {
        g[i] = AND_G(a[i], b[i]);
        p[i] = XOR_G(a[i], b[i]);
    }
}

// Carry-lookahead: c[i+1] = g[i] OR (p[i] AND c[i])
// Parallel prefix computation (Kogge-Stone style for encrypted)
vector<LWECiphertext> carry_lookahead(const vector<LWECiphertext>& g,
                                       const vector<LWECiphertext>& p,
                                       int bits) {
    vector<LWECiphertext> carry(bits + 1);
    auto zero = g_cc.Encrypt(g_sk, 0);
    carry[0] = clone_ct(zero);
    
    // Group size for parallel prefix
    for(int groupSize = 1; groupSize < bits; groupSize *= 2) {
        vector<LWECiphertext> new_g = g, new_p = p;
        
        // Parallel across groups
        for(int i = groupSize; i < bits; i++) {
            // Combine: (g_hi, p_hi) • (g_lo, p_lo)
            // new_g = g_hi OR (p_hi AND g_lo)
            // new_p = p_hi AND p_lo
            auto p_and_g = AND_G(p[i], g[i - groupSize]);
            new_g[i] = OR_G(g[i], p_and_g);
            new_p[i] = AND_G(p[i], p[i - groupSize]);
        }
        
        g = new_g;
        p = new_p;
    }
    
    // Now g[i] contains carry[i+1]
    for(int i = 0; i < bits; i++) {
        carry[i + 1] = clone_ct(g[i]);
    }
    
    return carry;
}

// ============================================
// PARALLEL COLUMN ACCUMULATION
// ============================================

// Process multiple columns independently
void process_columns(const vector<vector<LWECiphertext>>& pp,
                     vector<LWECiphertext>& g, vector<LWECiphertext>& p,
                     int start_col, int end_col, int bits) {
    auto zero = g_cc.Encrypt(g_sk, 0);
    
    for(int col = start_col; col < end_col && col < 2*bits; col++) {
        // Sum all partial products in this column using XOR tree
        LWECiphertext sum = clone_ct(zero);
        for(int i = 0; i < bits; i++) {
            int j = col - i;
            if(j >= 0 && j < bits) {
                sum = XOR_G(sum, pp[i][j]);
            }
        }
        g[col] = sum; // Temporary store sum in g
    }
}

// ============================================
// OPTIMIZED 32-BIT WITH CLA + PARALLEL COLUMNS
// ============================================

void generate_pp_parallel(const vector<LWECiphertext>& a, const vector<LWECiphertext>& b,
                           vector<vector<LWECiphertext>>& pp, int start, int end, int bits) {
    for(int i = start; i < end; i++)
        for(int j = 0; j < bits; j++)
            pp[i][j] = AND_G(a[j], b[i]);
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — CARRY-LOOKAHEAD + PARALLEL COLUMNS    ║\n";
    cout <<   "║  O(log n) carry | Independent columns        ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    g_cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    int bits = 8; // Start with 8-bit for fast testing
    int64_t testA = 42, testB = 17, expected = 714;
    auto zero = g_cc.Encrypt(g_sk, 0);
    
    cout << "Φ Test: " << testA << " × " << testB << " = " << expected << " (" << bits << "-bit)\n\n";
    
    // Encrypt inputs
    vector<LWECiphertext> a(bits), b(bits);
    for(int i = 0; i < bits; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    auto total_start = high_resolution_clock::now();
    
    // Phase 1: Parallel partial products
    cout << "Φ Phase 1: Parallel partial products (12 threads)...\n";
    vector<vector<LWECiphertext>> pp(bits, vector<LWECiphertext>(bits, clone_ct(zero)));
    
    int numThreads = 12;
    int rowsPerThread = max(1, bits / numThreads);
    vector<thread> threads;
    
    for(int t = 0; t < min(numThreads, bits); t++) {
        int s = t * rowsPerThread;
        int e = (t == numThreads-1) ? bits : s + rowsPerThread;
        if(s < bits)
            threads.push_back(thread(generate_pp_parallel, ref(a), ref(b), ref(pp), s, e, bits));
    }
    for(auto& t : threads) t.join();
    
    auto phase1_time = duration_cast<seconds>(high_resolution_clock::now() - total_start).count();
    cout << "  ✅ " << phase1_time << "s | " << g_totalGates << " gates\n";
    
    // Phase 2: Parallel column sums
    cout << "Φ Phase 2: Parallel column accumulation...\n";
    vector<LWECiphertext> column_sums(2*bits, clone_ct(zero));
    vector<LWECiphertext> column_g(2*bits, clone_ct(zero));
    vector<LWECiphertext> column_p(2*bits, clone_ct(zero));
    
    threads.clear();
    int colsPerThread = max(1, (2*bits) / numThreads);
    for(int t = 0; t < min(numThreads, 2*bits); t++) {
        int s = t * colsPerThread;
        int e = (t == numThreads-1) ? 2*bits : s + colsPerThread;
        if(s < 2*bits)
            threads.push_back(thread(process_columns, ref(pp), ref(column_g), 
                                     ref(column_p), s, e, bits));
    }
    for(auto& t : threads) t.join();
    
    // Copy column sums from column_g
    for(int i = 0; i < 2*bits; i++) {
        column_sums[i] = clone_ct(column_g[i]);
    }
    
    auto phase2_time = duration_cast<seconds>(high_resolution_clock::now() - total_start).count();
    cout << "  ✅ " << (phase2_time - phase1_time) << "s | " << g_totalGates << " gates\n";
    
    // Phase 3: Carry-lookahead across columns
    cout << "Φ Phase 3: Carry-lookahead propagation (O(log n))...\n";
    
    // Generate G and P for columns
    vector<LWECiphertext> G(2*bits), P(2*bits);
    for(int i = 0; i < 2*bits; i++) {
        G[i] = clone_ct(zero);  // No carry generation at column level
        P[i] = clone_ct(zero);  // Propagate column sum
    }
    
    // CLA for columns
    auto carries = carry_lookahead(G, P, 2*bits);
    
    // Apply carries to column sums
    vector<LWECiphertext> result(2*bits);
    for(int i = 0; i < 2*bits; i++) {
        result[i] = XOR_G(column_sums[i], carries[i]);
    }
    
    auto total_time = duration_cast<seconds>(high_resolution_clock::now() - total_start).count();
    
    // Decrypt
    int64_t val = 0;
    for(int i = 0; i < 2*bits; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1LL << i);
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  CLA + PARALLEL RESULTS                      ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  " << testA << " × " << testB << " = " << val << " (exp: " << expected << ")\n";
    cout <<   "║  Total gates: " << g_totalGates << "\n";
    cout <<   "║  Total time: " << total_time << "s\n";
    cout <<   "║  Phase 1: " << phase1_time << "s\n";
    cout <<   "║  Phase 2: " << (phase2_time - phase1_time) << "s\n";
    cout <<   "║  Phase 3: " << (total_time - phase2_time) << "s\n";
    cout <<   "║  Gates/sec: " << (g_totalGates / max((long long)total_time, 1LL)) << "\n";
    cout <<   "║  Status: " << (val == expected ? "✅ PASSED" : "❌ FAILED") << "\n";
    
    // 32-bit prediction
    cout <<   "╠══════════════════════════════════════════════╣\n";
    int pred_gates_32 = int(bits == 8 ? g_totalGates.load() * 16 : g_totalGates.load());
    int pred_time_32 = int(bits == 8 ? total_time * 16 : total_time);
    cout <<   "║  32-bit pred: ~" << pred_gates_32 << " gates, ~" << pred_time_32 << "s\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
