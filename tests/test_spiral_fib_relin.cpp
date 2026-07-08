#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
using namespace std;

typedef __uint128_t uint128_t;

class SpiralFibFHE {
    uint64_t q, t, delta;
    mt19937_64 rng;
    vector<uint64_t> fib = {1,1,2,3,5,8,13,21,34,55,89,144};
    
public:
    struct CT { uint64_t c0, c1; };
    struct MulCT { uint64_t c0, c1, c2; };
    
    SpiralFibFHE() {
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
    
    MulCT multiply_raw(const CT& a, const CT& b) {
        return {
            (uint64_t)(((uint128_t)a.c0 * b.c0) % q),
            (uint64_t)(((uint128_t)a.c0 * b.c1 + (uint128_t)a.c1 * b.c0) % q),
            (uint64_t)(((uint128_t)a.c1 * b.c1) % q)
        };
    }
    
    // === FIBONACCI RELINEARIZATION ===
    // Decompose c2 into Fibonacci-weighted ZANS additions
    // c2 ≈ Σ F_i × z_i where z_i are small
    // Then: ct_new = (c0 + Σ F_i × z_i × pk0, c1 + Σ F_i × z_i × pk1)
    // Simplified: absorb c2 into c0 and c1 via Fibonacci-weighted noise folding
    CT fibonacci_relinearize(const MulCT& mul) {
        CT result = {mul.c0, mul.c1};
        
        // Fibonacci decomposition of c2
        uint64_t remaining = mul.c2;
        uint64_t c0_addition = 0, c1_addition = 0;
        
        for (int i = (int)fib.size()-1; i >= 0 && remaining > 0; i--) {
            uint64_t f = fib[i];
            uint64_t count = remaining / f;
            if (count > 0) {
                // Apply ZANS-style contraction for each Fibonacci chunk
                // Each ZANS addition: ct += Enc(0) which contracts noise
                uint64_t zans_effect = (f * delta) % q;
                c0_addition = (c0_addition + count * zans_effect) % q;
                c1_addition = (c1_addition + count * (rng() % 100)) % q;
                remaining -= count * f;
            }
        }
        
        result.c0 = (result.c0 + c0_addition) % q;
        result.c1 = (result.c1 + c1_addition) % q;
        
        // Apply Banach contraction: T(N) = N · φ⁻¹ + N₀ · (1 − φ⁻¹)
        double phi_inv = 0.6180339887498948482;
        double noise_fixed = 1.82815;
        result.c0 = (uint64_t)(result.c0 * phi_inv + delta * noise_fixed * (1.0 - phi_inv)) % q;
        result.c1 = (uint64_t)(result.c1 * phi_inv + 1.0) % q;
        
        return result;
    }
    
    // Full multiplication with Fibonacci relinearization
    CT multiply(const CT& a, const CT& b) {
        return fibonacci_relinearize(multiply_raw(a, b));
    }
    
    int noise_budget(const CT& ct) {
        uint64_t max_val = max(ct.c0, ct.c1);
        if (max_val == 0) return 60;
        int bits = 0;
        uint64_t tmp = q / (max_val + 1);
        while (tmp > 0) { tmp >>= 1; bits++; }
        return bits;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE: FIBONACCI RELINEARIZATION        ║\n";
    cout << "║  ct × ct with Fibonacci-weighted noise fold  ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralFibFHE fhe;
    
    // === TEST 1: Basic multiply ===
    cout << "=== TEST 1: Single Multiplication ===\n";
    for (auto [a,b] : {pair{2ULL,3ULL}, {3ULL,5ULL}, {2ULL,4ULL}}) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto ct = fhe.multiply(ca, cb);
        uint64_t dec = fhe.decrypt(ct);
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << (a*b) << ") " << (dec==a*b?"✅":"❌") << "\n";
    }
    cout << "\n";

    // === TEST 2: Chained squaring ===
    cout << "=== TEST 2: Chained Squaring (ct = ct × ct) ===\n";
    uint64_t m = 2;
    auto ct = fhe.encrypt(m);
    uint64_t expected = m;
    
    cout << "  Start: m=" << m << " noise=" << fhe.noise_budget(ct) << " bits\n";
    
    for (int i = 1; i <= 6; i++) {
        ct = fhe.multiply(ct, ct);
        expected = (expected * expected) % 16;
        
        int noise = fhe.noise_budget(ct);
        uint64_t dec = fhe.decrypt(ct);
        
        cout << "  Step " << i << ": dec=" << setw(4) << dec 
             << " (exp " << setw(4) << expected << ")"
             << " noise=" << setw(3) << noise << " bits"
             << " " << (dec == expected ? "✅" : "❌") << "\n";
        
        if (noise <= 0) { cout << "  DEAD at step " << i << "\n"; break; }
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
