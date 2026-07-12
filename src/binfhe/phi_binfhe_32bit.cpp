// ΦΩ0 — BINFHE CT×CT: 32-BIT ENCRYPTED MULTIPLIER
// Scaled from 16-bit. Every gate bootstrapped.
// Estimated: ~30,000 gates, ~16-20 minutes
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext* g_cc = nullptr;
LWEPrivateKey g_sk = nullptr;
int gate_count = 0;

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

LWECiphertext enc_zero() {
    return g_cc->Encrypt(g_sk, 0);
}

LWECiphertext NAND_BT(const LWECiphertext& a, const LWECiphertext& b) {
    gate_count++;
    return g_cc->Bootstrap(g_cc->EvalBinGate(NAND, a, b));
}

LWECiphertext NOT_BT(const LWECiphertext& a) {
    return NAND_BT(a, clone_ct(a));
}

LWECiphertext AND_BT(const LWECiphertext& a, const LWECiphertext& b) {
    auto nand_ab = NAND_BT(a, b);
    return NAND_BT(nand_ab, clone_ct(nand_ab));
}

LWECiphertext OR_BT(const LWECiphertext& a, const LWECiphertext& b) {
    return NAND_BT(NOT_BT(a), NOT_BT(b));
}

LWECiphertext XOR_BT(const LWECiphertext& a, const LWECiphertext& b) {
    auto nand_ab = NAND_BT(a, b);
    return NAND_BT(NAND_BT(a, nand_ab), NAND_BT(b, nand_ab));
}

pair<LWECiphertext, LWECiphertext> half_adder(const LWECiphertext& a, const LWECiphertext& b) {
    return {XOR_BT(a, b), AND_BT(a, b)};
}

pair<LWECiphertext, LWECiphertext> full_adder(const LWECiphertext& a, const LWECiphertext& b, const LWECiphertext& cin) {
    auto [sum1, carry1] = half_adder(a, b);
    auto [sum2, carry2] = half_adder(sum1, cin);
    return {sum2, OR_BT(carry1, carry2)};
}

vector<LWECiphertext> multiply_32bit(const vector<LWECiphertext>& a, const vector<LWECiphertext>& b) {
    auto zero = enc_zero();
    int N = 32;
    
    // Partial products: 32 rows × 64 columns
    vector<vector<LWECiphertext>> partial(N, vector<LWECiphertext>(2*N));
    for(int i = 0; i < N; i++)
        for(int j = 0; j < 2*N; j++)
            partial[i][j] = clone_ct(zero);
    
    // Generate partial products
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
            partial[i][i+j] = AND_BT(a[j], b[i]);
    
    // Sum all rows
    vector<LWECiphertext> sum(2*N);
    for(int k = 0; k < 2*N; k++)
        sum[k] = clone_ct(partial[0][k]);
    
    for(int i = 1; i < N; i++) {
        LWECiphertext carry = nullptr;
        for(int j = 0; j < 2*N; j++) {
            if(carry == nullptr) {
                auto [s, c] = half_adder(sum[j], partial[i][j]);
                sum[j] = s;
                carry = c;
            } else {
                auto [s, c] = full_adder(sum[j], partial[i][j], carry);
                sum[j] = s;
                carry = c;
            }
        }
        // Progress indicator every 4 rows
        if(i % 4 == 0) {
            cout << "  Row " << i << "/32 complete...\n" << flush;
        }
    }
    
    return sum;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE CT×CT: 32-BIT MULTIPLIER        ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    
    g_sk = cc.KeyGen();
    cc.BTKeyGen(g_sk);
    g_cc = &cc;
    
    cout << "Φ BinFHE: GINX bootstrapping active.\n";
    cout << "Φ 32-bit multiplier ready.\n";
    cout << "Φ Estimated: ~30,000 gates, ~16-20 minutes\n\n";
    
    // Test: 42 × 17 = 714 (same as 16-bit, but now 32-bit)
    int x = 42, y = 17;
    
    vector<LWECiphertext> a(32), b(32);
    for(int i = 0; i < 32; i++) {
        a[i] = cc.Encrypt(g_sk, (x >> i) & 1);
        b[i] = cc.Encrypt(g_sk, (y >> i) & 1);
    }
    
    cout << "Φ " << x << " × " << y << " = ?\n";
    cout << "Φ Computing (this will take ~20 minutes)...\n\n";
    
    gate_count = 0;
    auto start = high_resolution_clock::now();
    auto result = multiply_32bit(a, b);
    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<seconds>(end - start).count();
    
    // Decrypt
    long long product = 0;
    for(int i = 63; i >= 0; i--) {
        LWEPlaintext bit;
        cc.Decrypt(g_sk, result[i], &bit);
        product = (product << 1) | bit;
    }
    
    cout << "\nΦ Result: " << product;
    cout << ((product == (long long)x * y) ? " ✅" : " ❌");
    cout << " | Gates: " << gate_count;
    cout << " | Time: " << elapsed << "s (" << elapsed/60.0 << " min)\n\n";
    
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    
    return 0;
}
