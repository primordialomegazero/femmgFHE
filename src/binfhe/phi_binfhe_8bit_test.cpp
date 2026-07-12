// ΦΩ0 — BINFHE 8-BIT MULTIPLICATION TEST
// Scaled optimized multiplier to 8-bit
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext g_cc;
LWEPrivateKey g_sk;
int g_gates = 0;

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

LWECiphertext NAND_P(const LWECiphertext& a, const LWECiphertext& b) {
    g_gates++;
    return g_cc.Bootstrap(g_cc.EvalBinGate(NAND, a, b));
}

LWECiphertext AND_P(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_P(a, b);
    return NAND_P(n, clone_ct(n));
}

LWECiphertext XOR_P(const LWECiphertext& a, const LWECiphertext& b) {
    auto n = NAND_P(a, b);
    return NAND_P(NAND_P(a, n), NAND_P(b, n));
}

vector<LWECiphertext> multiply_8bit(const vector<LWECiphertext>& a,
                                     const vector<LWECiphertext>& b) {
    auto zero = g_cc.Encrypt(g_sk, 0);
    vector<LWECiphertext> result(16, clone_ct(zero));
    
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            auto pp = AND_P(a[j], b[i]);
            int col = i + j;
            auto sum = XOR_P(result[col], pp);
            auto carry = AND_P(result[col], pp);
            result[col] = sum;
            
            int k = col + 1;
            while(k < 16) {
                auto new_sum = XOR_P(result[k], carry);
                carry = AND_P(result[k], carry);
                result[k] = new_sum;
                k++;
            }
        }
    }
    return result;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE 8-BIT MULTIPLIER TEST          ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    g_cc.GenerateBinFHEContext(TOY, false);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    // Test: 42 × 17 = 714
    int testA = 42, testB = 17, expected = 714;
    
    cout << "Φ Test: " << testA << " × " << testB << " = " << expected << "\n";
    cout << "Φ Encrypting inputs...\n";
    
    vector<LWECiphertext> a(8), b(8);
    for(int i = 0; i < 8; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    cout << "Φ Computing 8-bit multiplication...\n";
    cout << "Φ (This may take 3-5 minutes)\n";
    
    g_gates = 0;
    auto start = high_resolution_clock::now();
    auto result = multiply_8bit(a, b);
    auto elapsed = duration_cast<seconds>(high_resolution_clock::now() - start).count();
    
    cout << "\nΦ Decrypting result...\n";
    int val = 0;
    for(int i = 0; i < 16; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1 << i);
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  8-BIT MULTIPLICATION RESULTS                ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  " << testA << " × " << testB << " = " << val << " (expected: " << expected << ")\n";
    cout <<   "║  Gates: " << g_gates << "\n";
    cout <<   "║  Time: " << elapsed << " seconds\n";
    cout <<   "║  Gates/sec: " << (g_gates / max(elapsed, 1L)) << "\n";
    cout <<   "║  Status: " << (val == expected ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
