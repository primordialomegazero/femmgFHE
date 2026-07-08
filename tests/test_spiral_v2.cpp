#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_ntt.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v2 — DIRECT ENCRYPT/DECRYPT       ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralParams params;
    SpiralNTT ntt(params.poly_degree);
    uint64_t q = ntt.modulus();
    uint64_t t = params.plain_modulus;
    size_t N = params.poly_degree;

    cout << "q = " << q << " (" << (int)log2(q) << " bits)\n";
    cout << "t = " << t << " (" << (int)log2(t) << " bits)\n";
    cout << "N = " << N << "\n\n";

    // Generate small secret key s
    vector<int64_t> s(N, 0);
    s[0] = 1;  // Simple key for testing
    s[1] = -1;
    s[2] = 1;
    
    cout << "Secret key s: [1, -1, 1, 0, 0, ...]\n\n";

    // Generate random a, and compute pk = (-a·s + e, a)
    mt19937_64 rng(42);
    uniform_int_distribution<uint64_t> dist(0, q-1);
    
    vector<uint64_t> a(N, 0);
    for (size_t i = 0; i < N; i++) a[i] = dist(rng);
    
    // Convert s to uint64_t for NTT
    vector<uint64_t> s_vec(N, 0);
    for (size_t i = 0; i < N; i++) {
        s_vec[i] = (s[i] < 0) ? (q - (uint64_t)(-s[i])) : (uint64_t)s[i];
    }
    
    // a·s mod (x^N+1)
    auto a_s = ntt.multiply(a, s_vec);
    
    // pk0 = -a·s + e (with small noise e)
    vector<uint64_t> pk0(N, 0), pk1 = a;
    for (size_t i = 0; i < N; i++) {
        pk0[i] = (q - a_s[i]) % q;  // -a·s mod q
        // Add tiny noise
        pk0[i] = (pk0[i] + 1) % q;
    }
    
    cout << "=== ENCRYPTION (m=42) ===\n";
    uint64_t m = 42;
    
    // Encryption: c0 = m + pk0·u + e0, c1 = pk1·u + e1
    // Use small u for testing
    vector<uint64_t> u(N, 0);
    u[0] = 1;  // Simple randomness
    
    auto pk0_u = ntt.multiply(pk0, u);
    auto pk1_u = ntt.multiply(pk1, u);
    
    vector<uint64_t> c0(N, 0), c1(N, 0);
    c0[0] = (m + pk0_u[0] + 1) % q;  // +1 is tiny noise e0
    for (size_t i = 1; i < N; i++) c0[i] = (pk0_u[i] + 1) % q;
    for (size_t i = 0; i < N; i++) c1[i] = (pk1_u[i] + 1) % q;  // +1 is tiny noise e1
    
    cout << "  c0[0] = " << c0[0] << "\n";
    cout << "  c1[0] = " << c1[0] << "\n\n";
    
    // Decryption: m = c0 + c1·s mod q mod t
    cout << "=== DECRYPTION ===\n";
    auto c1_s = ntt.multiply(c1, s_vec);
    
    cout << "  (c1·s)[0] = " << c1_s[0] << "\n";
    cout << "  c0[0] + (c1·s)[0] = " << (c0[0] + c1_s[0]) << "\n";
    cout << "  (c0 + c1·s)[0] mod q = " << ((c0[0] + c1_s[0]) % q) << "\n";
    
    uint64_t dec = ((c0[0] + c1_s[0]) % q) % t;
    cout << "  Result mod t = " << dec << "\n";
    cout << "  Expected: " << m << " → " << (dec == m ? "✅ PASS!" : "❌ FAIL") << "\n\n";

    // Test with 10 different messages
    cout << "=== 10 MESSAGE TEST ===\n";
    int passed = 0;
    for (uint64_t msg = 1; msg <= 10; msg++) {
        c0[0] = (msg + pk0_u[0] + 1) % q;
        c1_s = ntt.multiply(c1, s_vec);
        dec = ((c0[0] + c1_s[0]) % q) % t;
        bool ok = (dec == msg);
        if (ok) passed++;
        cout << "  m=" << setw(2) << msg << " → dec=" << setw(2) << dec << " " << (ok ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  Result: " << passed << "/10 passed\n\n";
    
    // Additive homomorphism test
    cout << "=== ADDITIVE HOMOMORPHISM ===\n";
    uint64_t m1 = 15, m2 = 27;
    
    // Encrypt m1
    vector<uint64_t> c0_1(N, 0), c1_1(N, 0);
    c0_1[0] = (m1 + pk0_u[0] + 1) % q;
    for (size_t i = 1; i < N; i++) c0_1[i] = (pk0_u[i] + 1) % q;
    for (size_t i = 0; i < N; i++) c1_1[i] = (pk1_u[i] + 1) % q;
    
    // Encrypt m2
    vector<uint64_t> c0_2(N, 0), c1_2(N, 0);
    c0_2[0] = (m2 + pk0_u[0] + 1) % q;
    for (size_t i = 1; i < N; i++) c0_2[i] = (pk0_u[i] + 1) % q;
    for (size_t i = 0; i < N; i++) c1_2[i] = (pk1_u[i] + 1) % q;
    
    // Add ciphertexts
    vector<uint64_t> c0_sum(N, 0), c1_sum(N, 0);
    for (size_t i = 0; i < N; i++) {
        c0_sum[i] = (c0_1[i] + c0_2[i]) % q;
        c1_sum[i] = (c1_1[i] + c1_2[i]) % q;
    }
    
    // Decrypt sum
    auto c1s_sum = ntt.multiply(c1_sum, s_vec);
    uint64_t dec_sum = ((c0_sum[0] + c1s_sum[0]) % q) % t;
    
    cout << "  " << m1 << " + " << m2 << " = " << dec_sum << " (expected " << (m1+m2) << ") ";
    cout << (dec_sum == m1+m2 ? "✅" : "❌") << "\n\n";

    return 0;
}
