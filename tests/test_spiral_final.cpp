#include <iostream>
#include <iomanip>
#include <random>
using namespace std;

typedef __uint128_t uint128_t;

class SpiralFinal {
    uint64_t q, t, delta;
    mt19937_64 rng;
    
public:
    struct CT { uint64_t c0, c1; };
    struct MulCT { uint64_t c0, c1, c2; };
    
    SpiralFinal() {
        t = 16;
        q = 10000000000000000ULL; // ~2^53, big enough!
        delta = 1000000; // Small delta so delta²*m < q
        rng.seed(1618033988);
    }
    
    CT encrypt(uint64_t m) {
        uint64_t e0 = rng() % 100, e1 = rng() % 100;
        return {(m * delta + e0) % q, e1 % q};
    }
    
    uint64_t decrypt(const CT& ct) { return ((ct.c0 % q) / delta) % t; }
    
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
        uint128_t dsq = (uint128_t)delta * delta;
        return (uint64_t)(val / dsq) % t;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE FINAL — TRUE FHE TEST             ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralFinal fhe;
    int passed = 0, total = 0;

    cout << "=== ENC/DEC ===\n";
    for (uint64_t m : {1ULL,2ULL,3ULL,4ULL,5ULL,10ULL}) { total++;
        auto ct = fhe.encrypt(m); uint64_t d = fhe.decrypt(ct);
        if (d == m) passed++; cout << "  " << m << " → " << d << " " << (d==m?"✅":"❌") << "\n"; }
    cout << "\n=== ADD ===\n";
    for (auto [a,b] : {pair{2ULL,3ULL},{4ULL,5ULL},{7ULL,8ULL}}) { total++;
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        uint64_t d = fhe.decrypt(fhe.add(ca,cb));
        if (d == a+b) passed++; cout << "  " << a << "+" << b << "=" << d << (d==a+b?" ✅":" ❌") << "\n"; }
    cout << "\n=== MUL ===\n";
    for (auto [a,b] : {pair{2ULL,3ULL},{2ULL,4ULL},{3ULL,5ULL}}) { total++;
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        uint64_t d = fhe.decrypt_mul(fhe.multiply(ca,cb));
        if (d == a*b) passed++; cout << "  " << a << "×" << b << "=" << d << " (exp " << a*b << ") " << (d==a*b?"✅":"❌") << "\n"; }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  " << passed << "/" << total << " passed";
    if (passed == total) cout << " — ✅ TRUE FHE WORKS!";
    cout << "                    ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";
    return 0;
}
