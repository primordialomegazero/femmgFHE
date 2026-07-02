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
    
    // 1. Basic addition
    total++;
    PhiPoly a({1, 2, 3, 4, 5, 6, 7, 8}, MOD);
    PhiPoly b({8, 7, 6, 5, 4, 3, 2, 1}, MOD);
    PhiPoly c = a + b;
    bool add_ok = (c[0] == 9 && c[7] == 9);
    cout << "\n1. Addition: " << (add_ok ? "✅" : "❌") << endl;
    if (add_ok) pass++;
    
    // 2. Subtraction
    total++;
    PhiPoly d = a - b;
    bool sub_ok = (d[0] == -7 && d[7] == 7);
    cout << "2. Subtraction: " << (sub_ok ? "✅" : "❌") << endl;
    if (sub_ok) pass++;
    
    // 3. Multiplication (ring: x^8 + 1)
    total++;
    PhiPoly e = a * b;
    // Known: (1+2x+...+8x^7) * (8+7x+...+1x^7) mod (x^8+1)
    bool mul_ok = !e.is_zero();  // Result should be non-zero
    cout << "3. Multiplication (mod x^8+1): " << (mul_ok ? "✅" : "❌") << endl;
    cout << "   c[0]=" << e[0] << " c[4]=" << e[4] << endl;
    if (mul_ok) pass++;
    
    // 4. Scalar multiplication
    total++;
    PhiPoly f = a * 3;
    bool scalar_ok = (f[0] == 3 && f[7] == 24);
    cout << "4. Scalar ×3: " << (scalar_ok ? "✅" : "❌") << endl;
    if (scalar_ok) pass++;
    
    // 5. φ-scaling
    total++;
    PhiPoly g = a.phi_scale();
    bool phi_ok = (std::abs(g[0] - (int64_t)(1 * PHI)) <= 1);
    cout << "5. φ-scaling: " << (phi_ok ? "✅" : "❌") << endl;
    if (phi_ok) pass++;
    
    // 6. Noise generation
    total++;
    PhiPoly noise = phi_noise(8, MOD, 42);
    bool noise_ok = !noise.is_zero();
    cout << "6. φ-noise generation: " << (noise_ok ? "✅" : "❌") << endl;
    cout << "   noise[0]=" << noise[0] << " norm=" << noise.norm() << endl;
    if (noise_ok) pass++;
    
    // 7. Random polynomial
    total++;
    PhiPoly r1 = phi_random(8, MOD, 123);
    PhiPoly r2 = phi_random(8, MOD, 456);
    bool rand_ok = (r1 != r2);
    cout << "7. φ-random polynomials differ: " << (rand_ok ? "✅" : "❌") << endl;
    if (rand_ok) pass++;
    
    // 8. Ring homomorphism: (a+b)*c = a*c + b*c
    total++;
    PhiPoly lhs = (a + b) * c;
    PhiPoly rhs = (a * c) + (b * c);
    bool ring_ok = (lhs == rhs);
    cout << "8. Ring homomorphism (a+b)*c = a*c+b*c: " << (ring_ok ? "✅" : "❌") << endl;
    if (ring_ok) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  PHI-POLYNOMIAL: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
