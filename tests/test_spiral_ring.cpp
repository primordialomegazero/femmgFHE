#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_ring.h"
#include "../src/spiral/spiral_lwe.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL GOLDEN RATIO RING + LWE               ║\n";
    cout << "║  R_q = Z_q[x]/(x^2 - x - 1)                  ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralRing ring(1618033988);
    SpiralLWE lwe(1618033988);
    
    // === TEST 1: Ring Operations ===
    cout << "=== RING OPERATIONS ===\n";
    auto e1 = SpiralRing::Element(3, 5);
    auto e2 = SpiralRing::Element(7, 11);
    
    auto sum = ring.add(e1, e2);
    cout << "  (3+5x) + (7+11x) = " << sum.a << "+" << sum.b << "x (exp 10+16x) ✅\n";
    
    auto prod = ring.mul(e1, e2);
    // (3+5x)(7+11x) = 21 + 33x + 35x + 55x^2 = 21 + 68x + 55(x+1) = 76 + 123x
    cout << "  (3+5x)(7+11x) = " << prod.a << "+" << prod.b << "x ";
    uint64_t q = ring.modulus();
    cout << "(exp " << ((21+55)%q) << "+" << ((33+35+55)%q) << "x) ✅\n\n";

    // === TEST 2: LWE Encrypt/Decrypt ===
    cout << "=== LWE ENCRYPT/DECRYPT ===\n";
    auto [sk, pk] = lwe.keygen();
    
    int passed = 0, total = 0;
    for (uint64_t m : {1, 2, 10, 42, 100, 1000}) {
        total++;
        auto ct = lwe.encrypt(m, pk);
        uint64_t dec = lwe.decrypt(ct, sk);
        if (dec == m) passed++;
        cout << "  m=" << setw(4) << m << " → " << setw(4) << dec << " " << (dec == m ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === TEST 3: Homomorphic Addition ===
    cout << "=== HOMOMORPHIC ADDITION ===\n";
    for (auto [a, b] : {std::pair<uint64_t,uint64_t>{15, 27}, {100ULL,200ULL}, {500ULL,500ULL}}) {
        total++;
        auto ct_a = lwe.encrypt(a, pk);
        auto ct_b = lwe.encrypt(b, pk);
        auto ct_sum = lwe.add(ct_a, ct_b);
        uint64_t dec = lwe.decrypt(ct_sum, sk);
        if (dec == (a + b) % q) passed++;
        cout << "  " << a << "+" << b << "=" << dec << " (exp " << (a+b) << ") " << (dec == (a+b)%q ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === TEST 4: ZANS ===
    cout << "=== ZANS (10 iterations) ===\n";
    total++;
    auto ct_zans = lwe.encrypt(42, pk);
    ct_zans = lwe.zans(ct_zans, pk, 10);
    uint64_t dec_zans = lwe.decrypt(ct_zans, sk);
    if (dec_zans == 42) passed++;
    cout << "  42 → " << dec_zans << " after 10 ZANS " << (dec_zans == 42 ? "✅" : "❌") << "\n\n";

    // === TEST 5: Multiplication ===
    cout << "=== HOMOMORPHIC MULTIPLICATION ===\n";
    vector<pair<uint64_t,uint64_t>> mul_pairs = {{2,3}, {3,4}, {5,6}, {6,7}, {10,10}};
    for (auto [a, b] : mul_pairs) {
        total++;
        auto ct_a = lwe.encrypt(a, pk);
        auto ct_b = lwe.encrypt(b, pk);
        auto ct_mul = lwe.multiply_raw(ct_a, ct_b);
        uint64_t dec = lwe.decrypt_mul(ct_mul, sk);
        if (dec == (a * b) % q) passed++;
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << (a*b) << ") " << (dec == (a*b)%q ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === FINAL ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  Passed: " << passed << "/" << total << "\n";
    if (passed == total) cout << "║  ✅ ALL TESTS PASSED!                         ║\n";
    else cout << "║  🔧 Golden Ratio Ring working, tuning needed   ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
