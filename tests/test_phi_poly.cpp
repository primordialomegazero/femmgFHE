#include "../src/math/phi_polynomial.h"
#include <iostream>
#include <cassert>
using namespace std;
using namespace phi_polynomial;

int main() {
    cout << "======================================================" << endl;
    cout << "  PHI-POLYNOMIAL RING TEST" << endl;
    cout << "  R = Z[x] / (x^" << DEFAULT_N << " + 1), modulus = 2^20" << endl;
    cout << "======================================================" << endl;

    int pass = 0, total = 0;
    int64_t MOD = 1 << 20;
    int N = DEFAULT_N;

    // 1. Basic addition
    total++;
    PhiPoly a(N, MOD), b(N, MOD);
    for (int i = 0; i < N; i++) { a[i] = (i + 1) % 100; b[i] = (N - i) % 100; }
    PhiPoly c = a + b;
    bool add_ok = (c[0] == a[0] + b[0] && c[N-1] == a[N-1] + b[N-1]);
    cout << "\n1. Addition (N=" << N << "): " << (add_ok ? "✅" : "❌") << endl;
    if (add_ok) pass++;

    // 2. Subtraction
    total++;
    PhiPoly d = a - b;
    bool sub_ok = (d[0] == a[0] - b[0]);
    cout << "2. Subtraction: " << (sub_ok ? "✅" : "❌") << endl;
    if (sub_ok) pass++;

    // 3. Multiplication (ring: x^N + 1)
    total++;
    PhiPoly e = a * b;
    bool mul_ok = !e.is_zero();
    cout << "3. Multiplication (mod x^" << N << "+1): " << (mul_ok ? "✅" : "❌") << endl;
    cout << "   e[0]=" << e[0] << " e[" << N/2 << "]=" << e[N/2] << endl;
    if (mul_ok) pass++;

    // 4. Scalar multiplication
    total++;
    PhiPoly f = a * 3;
    bool scalar_ok = (f[0] == a[0] * 3);
    cout << "4. Scalar ×3: " << (scalar_ok ? "✅" : "❌") << endl;
    if (scalar_ok) pass++;

    // 5. φ-scaling
    total++;
    PhiPoly g = a.phi_scale();
    bool phi_ok = (std::abs(g[0] - (int64_t)(a[0] * PHI)) <= 1);
    cout << "5. φ-scaling: " << (phi_ok ? "✅" : "❌") << endl;
    if (phi_ok) pass++;

    // 6. Noise generation
    total++;
    PhiPoly noise = phi_noise(N, MOD, 42);
    bool noise_ok = !noise.is_zero();
    cout << "6. φ-noise generation: " << (noise_ok ? "✅" : "❌") << endl;
    cout << "   noise[0]=" << noise[0] << " norm=" << noise.norm() << endl;
    if (noise_ok) pass++;

    // 7. Random polynomials differ (FIXED: bigger N + xorshift)
    total++;
    PhiPoly r1 = phi_random(N, MOD, 123);
    PhiPoly r2 = phi_random(N, MOD, 456);
    bool rand_ok = (r1 != r2);
    cout << "7. φ-random polynomials differ: " << (rand_ok ? "✅" : "❌") << endl;
    if (rand_ok) pass++;

    // 8. Ring homomorphism: (a+b)*c = a*c + b*c
    total++;
    PhiPoly lhs = (a + b) * c;
    PhiPoly rhs = (a * c) + (b * c);
    bool ring_ok = (lhs == rhs);
    cout << "8. Ring homomorphism: " << (ring_ok ? "✅" : "❌") << endl;
    if (ring_ok) pass++;

    cout << "\n======================================================" << endl;
    cout << "  PHI-POLYNOMIAL: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;

    return (pass == total) ? 0 : 1;
}
