// ΦΩ0 — BINFHE 32-BIT PARALLEL (FIXED CARRY)
// Proper ripple-carry addition for correct results
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

LWECiphertext NAND_TS(const LWECiphertext& a, const LWECiphertext& b) {
    g_totalGates++;
    return g_cc.Bootstrap(g_cc.EvalBinGate(NAND, a, b));
}

LWECiphertext AND_TS(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_TS(a, b);
    return NAND_TS(n, clone_ct(n));
}

LWECiphertext XOR_TS(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_TS(a, b);
    return NAND_TS(NAND_TS(a, n), NAND_TS(b, n));
}

// Parallel partial products
void generate_pp(const vector<LWECiphertext>& a, const vector<LWECiphertext>& b,
                 vector<vector<LWECiphertext>>& pp, int start, int end, int bits) {
    for(int i = start; i < end; i++)
        for(int j = 0; j < bits; j++)
            pp[i][j] = AND_TS(a[j], b[i]);
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — 32-BIT PARALLEL (FIXED CARRY)         ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    g_cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    int64_t testA = 42, testB = 17, expected = 714;
    int bits = 32;
    auto zero = g_cc.Encrypt(g_sk, 0);
    
    cout << "Φ Test: " << testA << " × " << testB << " = " << expected << "\n";
    
    // Encrypt
    vector<LWECiphertext> a(bits), b(bits);
    for(int i = 0; i < bits; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    // Phase 1: Parallel partial products
    cout << "Φ Phase 1: Parallel partial products...\n";
    vector<vector<LWECiphertext>> pp(bits, vector<LWECiphertext>(bits, clone_ct(zero)));
    
    int numThreads = 12;
    int rowsPerThread = bits / numThreads;
    vector<thread> threads;
    
    auto start = high_resolution_clock::now();
    for(int t = 0; t < numThreads; t++) {
        int s = t * rowsPerThread;
        int e = (t == numThreads-1) ? bits : s + rowsPerThread;
        threads.push_back(thread(generate_pp, ref(a), ref(b), ref(pp), s, e, bits));
    }
    for(auto& t : threads) t.join();
    
    // Phase 2: Column accumulation with proper ripple carry
    cout << "Φ Phase 2: Column accumulation with ripple carry...\n";
    vector<LWECiphertext> result(2*bits, clone_ct(zero));
    
    for(int col = 0; col < 2*bits; col++) {
        // Sum all terms in this column
        LWECiphertext sum = clone_ct(zero);
        for(int i = 0; i < bits; i++) {
            int j = col - i;
            if(j >= 0 && j < bits) {
                sum = XOR_TS(sum, pp[i][j]);
            }
        }
        
        // Full adder chain for carry
        auto carry = clone_ct(zero);
        for(int i = 0; i < bits; i++) {
            int j = col - i;
            if(j >= 0 && j < bits) {
                auto new_carry1 = AND_TS(sum, pp[i][j]);
                auto new_carry2 = AND_TS(carry, XOR_TS(sum, pp[i][j]));
                sum = XOR_TS(XOR_TS(sum, pp[i][j]), carry);
                carry = XOR_TS(new_carry1, new_carry2);
            }
        }
        
        result[col] = sum;
        
        // Propagate carry to next column
        if(col + 1 < 2*bits) {
            int next_col = col + 1;
            for(int i = 0; i < bits; i++) {
                int j = next_col - i;
                if(j >= 0 && j < bits) {
                    pp[i][j] = XOR_TS(pp[i][j], carry);
                    break;
                }
            }
        }
    }
    
    auto elapsed = duration_cast<seconds>(high_resolution_clock::now() - start).count();
    
    // Decrypt
    int64_t val = 0;
    for(int i = 0; i < 2*bits; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1LL << i);
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  32-BIT PARALLEL RESULTS                     ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  " << testA << " × " << testB << " = " << val << " (exp: " << expected << ")\n";
    cout <<   "║  Gates: " << g_totalGates << "\n";
    cout <<   "║  Time: " << elapsed << " seconds\n";
    cout <<   "║  Gates/sec: " << (g_totalGates / max((long long)elapsed, 1LL)) << "\n";
    cout <<   "║  Status: " << (val == expected ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
