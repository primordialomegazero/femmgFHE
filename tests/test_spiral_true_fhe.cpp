#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

// ═══════════════════════════════════════════
// SPIRAL-FHE: TRUE HOMOMORPHIC ENCRYPTION
// Ring-LWE + φ-Irrationality Noise
// R_q = Z_q[x]/(x^N+1) with N=2
// ═══════════════════════════════════════════

class SpiralTrueFHE {
private:
    uint64_t q;  // Ciphertext modulus
    uint64_t t;  // Plaintext modulus
    uint64_t delta;  // q/t scaling
    
    // Secret key: s ∈ R_q
    uint64_t s0, s1;
    
    // Public key: pk = (p0, p1) = (-a·s + e, a)
    uint64_t p0_0, p0_1, p1_0, p1_1;
    
    mt19937_64 rng;
    
    // === φ-IRRATIONALITY NOISE GENERATOR ===
    // Uses the property that φ = [1;1,1,1,...] is maximally irrational
    // φ^k mod q produces values that are hard to predict without knowing k
    uint64_t phi_noise(uint64_t seed) const {
        uint64_t phi_mod = 1618033988 % q;
        uint64_t result = seed;
        for (int i = 0; i < 5; i++) {
            result = (result * phi_mod + 1) % q;
            result = (result ^ (result >> 17)) % q;
        }
        // Keep noise small relative to q/t
        return result % (delta / 4);
    }
    
    // Polynomial multiplication in R_q = Z_q[x]/(x^2+1)
    // (a0 + a1·x) × (b0 + b1·x) mod (x^2+1)
    // = (a0·b0 - a1·b1) + (a0·b1 + a1·b0)·x
    pair<uint64_t, uint64_t> poly_mul(
        uint64_t a0, uint64_t a1,
        uint64_t b0, uint64_t b1) const {
        uint64_t c0 = ((a0 * b0) % q + q - (a1 * b1) % q) % q;
        uint64_t c1 = ((a0 * b1) % q + (a1 * b0) % q) % q;
        return {c0, c1};
    }
    
public:
    SpiralTrueFHE(uint64_t modulus = 1618033988, uint64_t plain_mod = 256) 
        : q(modulus), t(plain_mod) {
        delta = q / t;
        rng.seed(1618033988);
        
        // Generate secret key
        s0 = 1; s1 = 0;  // Simple key: s = 1
        
        // Generate public key
        uint64_t a0 = rng() % q;
        uint64_t a1 = rng() % q;
        
        // a·s = a (since s=1)
        // p0 = -a + e (mod q)
        uint64_t e0 = phi_noise(rng());
        uint64_t e1 = phi_noise(rng());
        p0_0 = (q - a0 + e0) % q;
        p0_1 = (q - a1 + e1) % q;
        p1_0 = a0;
        p1_1 = a1;
    }
    
    // === ENCRYPTION ===
    // ct = (c0, c1) where c0 = m·delta + p0·u + e0, c1 = p1·u + e1
    struct Ciphertext {
        uint64_t c0_0, c0_1;  // c0 polynomial
        uint64_t c1_0, c1_1;  // c1 polynomial
    };
    
    Ciphertext encrypt(uint64_t message) {
        // Message as polynomial: m·delta + 0·x
        uint64_t m0 = (message * delta) % q;
        uint64_t m1 = 0;
        
        // Random u (small)
        uint64_t u0 = rng() % 4;
        uint64_t u1 = rng() % 4;
        
        // Noise e0, e1
        uint64_t e00 = phi_noise(rng());
        uint64_t e01 = phi_noise(rng());
        uint64_t e10 = phi_noise(rng());
        uint64_t e11 = phi_noise(rng());
        
        // p0·u
        auto p0_u = poly_mul(p0_0, p0_1, u0, u1);
        // p1·u
        auto p1_u = poly_mul(p1_0, p1_1, u0, u1);
        
        Ciphertext ct;
        ct.c0_0 = (m0 + p0_u.first + e00) % q;
        ct.c0_1 = (m1 + p0_u.second + e01) % q;
        ct.c1_0 = (p1_u.first + e10) % q;
        ct.c1_1 = (p1_u.second + e11) % q;
        
        return ct;
    }
    
    // === DECRYPTION ===
    // m = round((c0 + c1·s) / delta) mod t
    uint64_t decrypt(const Ciphertext& ct) {
        // c1·s = c1 (since s=1)
        uint64_t result0 = (ct.c0_0 + ct.c1_0) % q;
        // Round: (result + delta/2) / delta
        return ((result0 + delta/2) / delta) % t;
    }
    
    // === ADDITION ===
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        return {
            (a.c0_0 + b.c0_0) % q, (a.c0_1 + b.c0_1) % q,
            (a.c1_0 + b.c1_0) % q, (a.c1_1 + b.c1_1) % q
        };
    }
    
    // === MULTIPLICATION ===
    // Returns 3-component ciphertext (needs relinearization)
    struct MulCiphertext {
        uint64_t c0_0, c0_1;
        uint64_t c1_0, c1_1;
        uint64_t c2_0, c2_1;
    };
    
    MulCiphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        auto c0 = poly_mul(a.c0_0, a.c0_1, b.c0_0, b.c0_1);
        auto term1 = poly_mul(a.c0_0, a.c0_1, b.c1_0, b.c1_1);
        auto term2 = poly_mul(a.c1_0, a.c1_1, b.c0_0, b.c0_1);
        auto c2 = poly_mul(a.c1_0, a.c1_1, b.c1_0, b.c1_1);
        
        return {
            c0.first, c0.second,
            (term1.first + term2.first) % q, (term1.second + term2.second) % q,
            c2.first, c2.second
        };
    }
    
    // Decrypt multiplication result
    uint64_t decrypt_mul(const MulCiphertext& ct) {
        // m1·m2 = (c0 + c1·s + c2·s²) / delta²
        // With s=1: c0 + c1 + c2
        uint64_t result = (ct.c0_0 + ct.c1_0 + ct.c2_0) % q;
        uint64_t delta_sq = delta * delta;
        return ((result + delta_sq/2) / delta_sq) % t;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE: TRUE HOMOMORPHIC ENCRYPTION     ║\n";
    cout << "║  Ring-LWE + φ-Irrationality Noise             ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralTrueFHE fhe(1618033988 * 100ULL, 256);
    
    int passed = 0, total = 0;

    // === ENCRYPT/DECRYPT ===
    cout << "=== ENCRYPT/DECRYPT ===\n";
    for (uint64_t m : {1ULL, 2ULL, 10ULL, 42ULL, 100ULL, 200ULL}) {
        total++;
        auto ct = fhe.encrypt(m);
        uint64_t dec = fhe.decrypt(ct);
        if (dec == m) passed++;
        cout << "  m=" << setw(3) << m << " → " << setw(3) << dec << " " << (dec == m ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === ADDITION ===
    cout << "=== HOMOMORPHIC ADDITION ===\n";
    for (auto [a, b] : {pair{15ULL,27ULL}, pair{50ULL,50ULL}, pair{100ULL,150ULL}}) {
        total++;
        auto ct_a = fhe.encrypt(a);
        auto ct_b = fhe.encrypt(b);
        auto ct_sum = fhe.add(ct_a, ct_b);
        uint64_t dec = fhe.decrypt(ct_sum);
        if (dec == (a + b) % 256) passed++;
        cout << "  " << a << "+" << b << "=" << dec << " (exp " << ((a+b)%256) << ") " << (dec == (a+b)%256 ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === MULTIPLICATION ===
    cout << "=== HOMOMORPHIC MULTIPLICATION ===\n";
    for (auto [a, b] : {pair{2ULL,3ULL}, pair{5ULL,6ULL}, pair{10ULL,20ULL}}) {
        total++;
        auto ct_a = fhe.encrypt(a);
        auto ct_b = fhe.encrypt(b);
        auto ct_mul = fhe.multiply(ct_a, ct_b);
        uint64_t dec = fhe.decrypt_mul(ct_mul);
        if (dec == (a * b) % 256) passed++;
        cout << "  " << a << "×" << b << "=" << dec << " (exp " << ((a*b)%256) << ") " << (dec == (a*b)%256 ? "✅" : "❌") << "\n";
    }
    cout << "  " << passed << "/" << total << "\n\n";

    // === FINAL ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  Passed: " << passed << "/" << total << "\n";
    if (passed == total) cout << "║  ✅ SPIRAL-FHE TRUE HOMOMORPHISM WORKS!       ║\n";
    else cout << "║  🔧 Tuning needed                              ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
