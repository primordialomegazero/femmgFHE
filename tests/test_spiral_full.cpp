#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_keygen.h"
#include "../src/spiral/spiral_encrypt.h"
#include "../src/spiral/spiral_decrypt.h"
#include "../src/spiral/spiral_add.h"
#include "../src/spiral/spiral_mul.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v3 — FULL TEST (ADD + MUL)        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    cout << "  " << declare::SCHEME_NAME << "\n";
    cout << "  " << declare::SCHEME_BASIS << "\n";
    cout << "  " << declare::SOURCE << "\n\n";

    SpiralParams params;
    SpiralKeyGen keygen(params);
    SpiralEncrypt encrypt(params);
    SpiralDecrypt decrypt(params);
    SpiralAdd add(params);
    SpiralMul mul(params);

    auto kp = keygen.generate();
    auto enc_zero = encrypt.encrypt_zero(kp.pk);

    int passed = 0, total = 0;

    // === TEST 1: Encrypt/Decrypt ===
    cout << "=== TEST 1: Encrypt/Decrypt ===\n";
    for (uint64_t m = 1; m <= 10; m++) {
        total++;
        auto ct = encrypt.encrypt(m, kp.pk);
        uint64_t dec = decrypt.decrypt(ct, kp.sk);
        if (dec == m) passed++;
        cout << "  m=" << setw(2) << m << " → " << setw(2) << dec << " " << (dec == m ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === TEST 2: Homomorphic Addition ===
    cout << "=== TEST 2: Homomorphic Addition ===\n";
    vector<pair<uint64_t,uint64_t>> pairs = {{15,27}, {100,200}, {500,500}, {1000,1}, {42,58}};
    for (auto [a, b] : pairs) {
        total++;
        auto ct_a = encrypt.encrypt(a, kp.pk);
        auto ct_b = encrypt.encrypt(b, kp.pk);
        auto ct_sum = add.add(ct_a, ct_b);
        uint64_t dec = decrypt.decrypt(ct_sum, kp.sk);
        if (dec == a + b) passed++;
        cout << "  " << a << "+" << b << "=" << dec << " (exp " << (a+b) << ") " << (dec == a+b ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === TEST 3: ZANS ===
    cout << "=== TEST 3: ZANS (100 iterations) ===\n";
    uint64_t m_zans = 42;
    total++;
    auto ct_zans = encrypt.encrypt(m_zans, kp.pk);
    for (int i = 0; i < 100; i++) {
        ct_zans = add.zans(ct_zans, enc_zero);
    }
    uint64_t dec_zans = decrypt.decrypt(ct_zans, kp.sk);
    if (dec_zans == m_zans) passed++;
    cout << "  Original: " << m_zans << " → After 100 ZANS: " << dec_zans << " " << (dec_zans == m_zans ? "✅" : "❌") << "\n\n";

    // === TEST 4: Homomorphic Multiplication ===
    cout << "=== TEST 4: Homomorphic Multiplication ===\n";
    vector<pair<uint64_t,uint64_t>> mul_pairs = {{6,7}, {3,5}, {10,10}, {2,3}, {7,8}};
    for (auto [a, b] : mul_pairs) {
        total++;
        auto ct_a = encrypt.encrypt(a, kp.pk);
        auto ct_b = encrypt.encrypt(b, kp.pk);
        auto ct_mul = mul.multiply_raw(ct_a, ct_b);
        uint64_t dec = mul.decrypt_mul(ct_mul, kp.sk);
        if (dec == a * b) passed++;
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << (a*b) << ") " << (dec == a*b ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === FINAL ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v3 — FINAL SCORE                  ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  Passed: " << passed << "/" << total << "\n";
    if (passed == total) {
        cout << "║  ✅ ALL TESTS PASSED!                         ║\n";
        cout << "║  Spiral-FHE is a WORKING FHE scheme!          ║\n";
    }
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
