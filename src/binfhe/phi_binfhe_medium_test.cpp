// ΦΩ0 — BINFHE TOY vs MEDIUM vs STD128
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

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

vector<LWECiphertext> multiply_4bit(const vector<LWECiphertext>& a,
                                     const vector<LWECiphertext>& b) {
    auto zero = g_cc.Encrypt(g_sk, 0);
    vector<LWECiphertext> result(8, clone_ct(zero));
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            auto pp = AND_P(a[j], b[i]);
            int col = i + j;
            auto sum = XOR_P(result[col], pp);
            auto carry = AND_P(result[col], pp);
            result[col] = sum;
            int k = col + 1;
            while(k < 8) {
                auto new_sum = XOR_P(result[k], carry);
                carry = AND_P(result[k], carry);
                result[k] = new_sum;
                k++;
            }
        }
    }
    return result;
}

void run_test(const string& mode, BINFHE_PARAMSET param, int testA, int testB, int expected) {
    cout << "\n▶ " << mode << ": " << testA << " × " << testB << " = " << expected << "\n";
    
    g_cc.GenerateBinFHEContext(param, GINX);
    g_sk = g_cc.KeyGen();
    g_cc.BTKeyGen(g_sk);
    
    vector<LWECiphertext> a(4), b(4);
    for(int i = 0; i < 4; i++) {
        a[i] = g_cc.Encrypt(g_sk, (testA >> i) & 1);
        b[i] = g_cc.Encrypt(g_sk, (testB >> i) & 1);
    }
    
    g_gates = 0;
    auto start = high_resolution_clock::now();
    auto result = multiply_4bit(a, b);
    auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    
    int val = 0;
    for(int i = 0; i < 8; i++) {
        LWEPlaintext bit;
        g_cc.Decrypt(g_sk, result[i], &bit);
        if(bit == 1) val |= (1 << i);
    }
    
    double gatesPerSec = g_gates * 1000.0 / elapsed;
    cout << "  Gates: " << g_gates << " | Time: " << elapsed << "ms | Gates/sec: " 
         << fixed << setprecision(1) << gatesPerSec << " | " << val 
         << (val == expected ? " ✅" : " ❌") << "\n";
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE TOY vs MEDIUM vs STD128        ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    cout << "Φ 4-bit: 3 × 14 = 42\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    run_test("TOY", TOY, 3, 14, 42);
    run_test("MEDIUM", MEDIUM, 3, 14, 42);
    run_test("STD128", STD128, 3, 14, 42);
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    return 0;
}
