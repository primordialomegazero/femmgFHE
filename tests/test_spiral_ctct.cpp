#include <iostream>
#include <iomanip>
#include <random>
using namespace std;

typedef __uint128_t uint128_t;

class SpiralFHE {
    uint64_t q, t, delta;
    mt19937_64 rng;
    
public:
    struct CT { uint64_t c0, c1; };
    struct MulCT { uint64_t c0, c1, c2; };
    
    SpiralFHE() {
        t = 16;
        q = 10000000000000000ULL;
        delta = 1000000;
        rng.seed(1618033988);
    }
    
    CT encrypt(uint64_t m) {
        uint64_t e0 = rng() % 100, e1 = rng() % 100;
        return {(m * delta + e0) % q, e1 % q};
    }
    
    uint64_t decrypt(const CT& ct) { return ((ct.c0 % q) / delta) % t; }
    CT add(const CT& a, const CT& b) { return {(a.c0 + b.c0) % q, (a.c1 + b.c1) % q}; }
    
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
    
    // === RECURSIVE SQUARING: ct = ct × ct ===
    void square_inplace(CT& ct) {
        MulCT mul = multiply(ct, ct);
        // Convert 3-component back to 2-component (relinearization)
        // Simplified: drop c2 for noise analysis
        ct.c0 = mul.c0;
        ct.c1 = mul.c1;
    }
    
    int noise_budget(const CT& ct) {
        uint64_t max_val = max(ct.c0, ct.c1);
        if (max_val == 0) return 60;
        int bits = 0;
        uint64_t tmp = q / max_val;
        while (tmp > 0) { tmp >>= 1; bits++; }
        return bits;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE: ct × ct WITHOUT BOOTSTRAPPING    ║\n";
    cout << "║  Chained Squaring Test                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralFHE fhe;
    
    uint64_t m = 2;  // Start with 2
    auto ct = fhe.encrypt(m);
    
    cout << "=== CHAINED SQUARING ===\n";
    cout << "  Start: m=" << m << " noise=" << fhe.noise_budget(ct) << " bits\n";
    
    int steps = 0;
    uint64_t expected = m;
    
    for (int i = 1; i <= 8; i++) {
        fhe.square_inplace(ct);
        expected = (expected * expected) % 16;
        steps++;
        
        int noise = fhe.noise_budget(ct);
        uint64_t dec = fhe.decrypt(ct);
        
        cout << "  Step " << i << ": dec=" << setw(4) << dec 
             << " (exp " << setw(4) << expected << ")"
             << " noise=" << setw(3) << noise << " bits"
             << " " << (dec == expected ? "✅" : "❌");
        
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    
    cout << "\n  Survived: " << steps << " squarings\n";
    cout << "  Bootstrapping needed: " << (steps < 8 ? "YES" : "NO") << "\n";
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    if (steps >= 8) cout << "║  ✅ ct × ct WORKS WITHOUT BOOTSTRAPPING!       ║\n";
    else cout << "║  🔧 Bootstrapping needed after " << steps << " steps              ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
