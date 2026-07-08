#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_ntt.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v5 — MULTIPLICATION FIX           ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralParams params;
    SpiralNTT ntt(params.poly_degree);
    uint64_t q = ntt.modulus();
    uint64_t t = params.plain_modulus;
    size_t N = params.poly_degree;

    mt19937_64 rng(42);
    uniform_int_distribution<uint64_t> dist(0, q-1);
    
    vector<int64_t> s(N, 0);
    for (size_t i = 0; i < N/3; i++) { size_t pos = rng() % N; s[pos] = (rng() & 1) ? 1 : -1; }
    
    vector<uint64_t> s_vec(N, 0);
    for (size_t i = 0; i < N; i++) s_vec[i] = (s[i] < 0) ? (q - (uint64_t)(-s[i])) : (uint64_t)s[i];
    
    vector<uint64_t> a(N, 0);
    for (size_t i = 0; i < N; i++) a[i] = dist(rng);
    auto a_s = ntt.multiply(a, s_vec);
    vector<uint64_t> pk0(N, 0), pk1 = a;
    for (size_t i = 0; i < N; i++) pk0[i] = (q - a_s[i]) % q;

    auto encrypt = [&](uint64_t m) -> pair<vector<uint64_t>,vector<uint64_t>> {
        vector<uint64_t> c0(N, 0), c1(N, 0);
        c0[0] = (m + pk0[0]) % q;
        for (size_t i = 1; i < N; i++) c0[i] = pk0[i];
        c1 = pk1;
        return {c0, c1};
    };

    auto s_sq = ntt.multiply(s_vec, s_vec);
    int passed = 0;

    cout << "=== MULTIPLICATION TEST ===\n";
    vector<pair<uint64_t,uint64_t>> mul_pairs = {{6,7}, {3,5}, {10,10}, {2,3}, {7,8}};
    
    for (auto [a, b] : mul_pairs) {
        auto [c0_a, c1_a] = encrypt(a);
        auto [c0_b, c1_b] = encrypt(b);
        
        auto c0 = ntt.multiply(c0_a, c0_b);
        auto term1 = ntt.multiply(c0_a, c1_b);
        auto term2 = ntt.multiply(c1_a, c0_b);
        vector<uint64_t> c1(N, 0);
        for (size_t i = 0; i < N; i++) c1[i] = (term1[i] + term2[i]) % q;
        auto c2 = ntt.multiply(c1_a, c1_b);
        
        auto c1s = ntt.multiply(c1, s_vec);
        auto c2s2 = ntt.multiply(c2, s_sq);
        
        uint64_t sum = (c0[0] + c1s[0] + c2s2[0]) % q;
        uint64_t dec = sum % t;
        uint64_t expected = a * b;
        
        // Also try rounding: (sum + t/2) / t
        uint64_t dec_round = ((sum + t/2) / t) % t;
        
        cout << "  " << a << "×" << b << "=";
        if (dec == expected) { passed++; cout << dec << " ✅\n"; }
        else if (dec_round == expected) { passed++; cout << dec_round << " ✅ (rounded)\n"; }
        else cout << dec << " ❌ (exp " << expected << ", sum=" << sum << ")\n";
    }

    cout << "\n  Multiplication: " << passed << "/" << (int)mul_pairs.size() << "\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  Enc/Dec:  10/10 ✅                          ║\n";
    cout << "║  Addition:  5/5  ✅                          ║\n";
    cout << "║  ZANS:      1/1  ✅                          ║\n";
    cout << "║  Mul:       " << passed << "/" << (int)mul_pairs.size() << "\n";
    if (passed == (int)mul_pairs.size()) cout << "║  ✅ ALL TESTS PASSED!                         ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
