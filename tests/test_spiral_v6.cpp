#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_ntt.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v6 — MUL WITH PROPER SCALING      ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralParams params;
    SpiralNTT ntt(params.poly_degree);
    uint64_t q = ntt.modulus();
    uint64_t t = params.plain_modulus;
    size_t N = params.poly_degree;
    uint64_t delta = q / t;  // Scaling factor

    cout << "q = " << q << " (" << (int)log2(q) << " bits)\n";
    cout << "t = " << t << " (" << (int)log2(t) << " bits)\n";
    cout << "delta = q/t = " << delta << "\n\n";

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
        c0[0] = (m * delta + pk0[0]) % q;  // SCALE message by delta!
        for (size_t i = 1; i < N; i++) c0[i] = pk0[i];
        c1 = pk1;
        return {c0, c1};
    };

    auto decrypt = [&](const vector<uint64_t>& c0, const vector<uint64_t>& c1) -> uint64_t {
        auto c1s = ntt.multiply(c1, s_vec);
        uint64_t val = (c0[0] + c1s[0]) % q;
        return (val + delta/2) / delta % t;  // Round to nearest
    };

    auto s_sq = ntt.multiply(s_vec, s_vec);
    int passed = 0;

    // Quick encrypt/decrypt test
    cout << "=== ENCRYPT/DECRYPT (scaled) ===\n";
    for (uint64_t m : {1, 2, 10, 42, 100}) {
        auto [c0, c1] = encrypt(m);
        uint64_t dec = decrypt(c0, c1);
        cout << "  m=" << m << " → " << dec << " " << (dec == m ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // Addition test
    cout << "=== ADDITION (scaled) ===\n";
    for (auto [a, b] : {pair{15ULL,27ULL}, {100ULL,200ULL}, {500ULL,500ULL}}) {
        auto [c0_a, c1_a] = encrypt(a);
        auto [c0_b, c1_b] = encrypt(b);
        vector<uint64_t> c0_sum(N, 0), c1_sum(N, 0);
        for (size_t i = 0; i < N; i++) {
            c0_sum[i] = (c0_a[i] + c0_b[i]) % q;
            c1_sum[i] = (c1_a[i] + c1_b[i]) % q;
        }
        uint64_t dec = decrypt(c0_sum, c1_sum);
        cout << "  " << a << "+" << b << "=" << dec << (dec == a+b ? " ✅" : " ❌") << "\n";
    }
    cout << "\n";

    // Multiplication test
    cout << "=== MULTIPLICATION (scaled) ===\n";
    vector<pair<uint64_t,uint64_t>> mul_pairs = {{2,3}, {3,4}, {5,6}, {6,7}, {10,10}};
    
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
        
        // Decrypt with scaling
        uint64_t val = (c0[0] + c1s[0] + c2s2[0]) % q;
        uint64_t dec = (val + delta*delta/2) / (delta*delta) % t;
        // For multiplication, divide by delta^2 since both inputs were scaled by delta
        
        uint64_t expected = a * b;
        if (dec == expected) passed++;
        
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << expected << ")";
        if (dec == expected) cout << " ✅";
        else cout << " ❌ (raw=" << val << ")";
        cout << "\n";
    }

    cout << "\n  Multiplication: " << passed << "/" << mul_pairs.size() << "\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  Mul: " << passed << "/" << mul_pairs.size() << "\n";
    if (passed == (int)mul_pairs.size()) cout << "║  ✅ ALL TESTS PASSED!                         ║\n";
    else cout << "║  🔧 Delta scaling needs tuning                ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
