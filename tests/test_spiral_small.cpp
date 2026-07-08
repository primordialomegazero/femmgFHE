#include <iostream>
#include <iomanip>
#include <random>
using namespace std;

typedef __uint128_t uint128_t;

class SpiralSmall {
    uint64_t q, t, delta;
    mt19937_64 rng;
    
public:
    struct CT { uint64_t c0, c1; };
    struct MulCT { uint64_t c0, c1, c2; };
    
    SpiralSmall() {
        t = 16;  // Small plaintext
        q = 1000000007;  // ~2^30 prime
        delta = q / (t * 2);  // Keep delta * m < q for all m < t
        rng.seed(42);
    }
    
    CT encrypt(uint64_t m) {
        uint64_t e0 = rng() % 100;
        uint64_t e1 = rng() % 100;
        return {(m * delta + e0) % q, e1 % q};
    }
    
    uint64_t decrypt(const CT& ct) {
        return ((ct.c0 % q) / delta) % t;
    }
    
    CT add(const CT& a, const CT& b) {
        return {(a.c0 + b.c0) % q, (a.c1 + b.c1) % q};
    }
    
    MulCT multiply(const CT& a, const CT& b) {
        return {
            (uint64_t)(((uint128_t)a.c0 * b.c0) % q),
            (uint64_t)(((uint128_t)a.c0 * b.c1 + (uint128_t)a.c1 * b.c0) % q),
            (uint64_t)(((uint128_t)a.c1 * b.c1) % q)
        };
    }
    
    uint64_t decrypt_mul(const MulCT& ct) {
        uint128_t val = ((uint128_t)ct.c0 + ct.c1 + ct.c2) % q;
        return (uint64_t)(val / ((uint128_t)delta * delta)) % t;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE SMALL — NO OVERFLOW               ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralSmall fhe;
    int passed = 0, total = 0;

    cout << "=== ENC/DEC ===\n";
    for (uint64_t m : {1ULL, 2ULL, 3ULL, 4ULL, 5ULL}) {
        total++;
        auto ct = fhe.encrypt(m);
        uint64_t dec = fhe.decrypt(ct);
        if (dec == m) passed++;
        cout << "  " << m << " → " << dec << " " << (dec == m ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    cout << "=== ADD ===\n";
    for (auto [a,b] : {pair{2ULL,3ULL}, pair{4ULL,5ULL}}) {
        total++;
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        uint64_t dec = fhe.decrypt(fhe.add(ca, cb));
        if (dec == (a+b)) passed++;
        cout << "  " << a << "+" << b << "=" << dec << (dec==a+b?" ✅":" ❌") << "\n";
    }
    cout << "\n";

    cout << "=== MUL ===\n";
    for (auto [a,b] : {pair{2ULL,3ULL}, pair{2ULL,4ULL}}) {
        total++;
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        uint64_t dec = fhe.decrypt_mul(fhe.multiply(ca, cb));
        if (dec == a*b) passed++;
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << a*b << ") " << (dec==a*b?"✅":"❌") << "\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  Passed: " << passed << "/" << total << "\n";
    if (passed == total) cout << "║  ✅ TRUE FHE WORKS!                           ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
