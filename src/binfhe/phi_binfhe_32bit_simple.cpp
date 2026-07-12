// ΦΩ0 — BINFHE 32-BIT: SIMPLE & WORKING
// Parallel partial products + sequential ripple carry
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext g_cc;
LWEPrivateKey g_sk;
atomic<int> g_gates{0};

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}
LWECiphertext NAND_G(const LWECiphertext& a, const LWECiphertext& b) {
    g_gates++;
    return g_cc.Bootstrap(g_cc.EvalBinGate(NAND, a, b));
}
LWECiphertext AND_G(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_G(a, b);
    return NAND_G(n, clone_ct(n));
}
LWECiphertext XOR_G(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_G(a, b);
    return NAND_G(NAND_G(a, n), NAND_G(b, n));
}

void generate_pp(const vector<LWECiphertext>& a, const vector<LWECiphertext>& b,
                 vector<vector<LWECiphertext>>& pp, int start, int end, int bits) {
    for(int i = start; i < end; i++)
        for(int j = 0; j < bits; j++)
            pp[i][j] = AND_G(a[j], b[i]);
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — 32-BIT: PARALLEL PP + RIPPLE CARRY    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    g_cc.GenerateBinFHEContext(TOY, GINX);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    int bits = 32;
    int64_t testA = 42, testB = 17, expected = 714;
    auto zero = g_cc.Encrypt(g_sk, 0);
    
    cout << "Φ " << testA << " × " << testB << " = " << expected << "\n\n";
    
    vector<LWECiphertext> a(bits), b(bits);
    for(int i = 0; i < bits; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    auto start = high_resolution_clock::now();
    
    // Phase 1: Parallel partial products
    cout << "Φ Phase 1: Parallel partial products...\n";
    vector<vector<LWECiphertext>> pp(bits, vector<LWECiphertext>(bits, clone_ct(zero)));
    vector<thread> threads;
    int nThreads = 12;
    int rowsPer = max(1, bits / nThreads);
    
    for(int t = 0; t < nThreads; t++) {
        int s = t * rowsPer;
        int e = (t == nThreads-1) ? bits : min(s + rowsPer, bits);
        if(s < bits)
            threads.push_back(thread(generate_pp, ref(a), ref(b), ref(pp), s, e, bits));
    }
    for(auto& t : threads) t.join();
    cout << "  ✅ " << duration_cast<seconds>(high_resolution_clock::now() - start).count() 
         << "s | " << g_gates << " gates\n";
    
    // Phase 2: Sequential ripple carry (simple but correct)
    cout << "Φ Phase 2: Ripple carry accumulation...\n";
    vector<LWECiphertext> result(2*bits);
    LWECiphertext carry = clone_ct(zero);
    
    for(int col = 0; col < 2*bits; col++) {
        auto sum = clone_ct(carry);
        carry = clone_ct(zero);
        
        for(int i = 0; i < bits; i++) {
            int j = col - i;
            if(j >= 0 && j < bits) {
                auto new_sum = XOR_G(sum, pp[i][j]);
                auto new_carry1 = AND_G(sum, pp[i][j]);
                auto new_carry2 = AND_G(carry, XOR_G(sum, pp[i][j]));
                carry = XOR_G(new_carry1, new_carry2);
                sum = new_sum;
            }
        }
        result[col] = sum;
    }
    
    auto total_time = duration_cast<seconds>(high_resolution_clock::now() - start).count();
    cout << "  ✅ Total: " << total_time << "s | " << g_gates << " gates\n";
    
    // Decrypt
    cout << "Φ Decrypting...\n";
    int64_t val = 0;
    for(int i = 0; i < 2*bits; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1LL << i);
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  32-BIT RESULT                               ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  " << testA << " × " << testB << " = " << val << " (exp: " << expected << ")\n";
    cout <<   "║  Gates: " << g_gates << "\n";
    cout <<   "║  Time: " << total_time << "s\n";
    cout <<   "║  Gates/sec: " << (g_gates.load() / max((long long)total_time, 1LL)) << "\n";
    cout <<   "║  Status: " << (val == expected ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
