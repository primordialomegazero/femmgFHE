#include <iostream>
#include <random>
using namespace std;

typedef __uint128_t uint128_t;

int main() {
    uint64_t q = 1000000007, t = 16;
    uint64_t delta = q / (t * 2);
    mt19937_64 rng(42);
    
    cout << "q=" << q << " t=" << t << " delta=" << delta << "\n";
    cout << "delta²=" << (uint64_t)(((uint128_t)delta * delta) % q) << "\n\n";
    
    uint64_t m1 = 2, m2 = 3;
    uint64_t e10 = rng()%100, e11 = rng()%100;
    uint64_t e20 = rng()%100, e21 = rng()%100;
    
    uint64_t c10 = (m1 * delta + e10) % q;
    uint64_t c11 = e11 % q;
    uint64_t c20 = (m2 * delta + e20) % q;
    uint64_t c21 = e21 % q;
    
    cout << "Enc(2): c0=" << c10 << " c1=" << c11 << " (raw=" << (m1*delta+e10) << ")\n";
    cout << "Enc(3): c0=" << c20 << " c1=" << c21 << " (raw=" << (m2*delta+e20) << ")\n\n";
    
    uint128_t mul_c0 = ((uint128_t)c10 * c20) % q;
    uint128_t mul_c1 = ((uint128_t)c10 * c21 + (uint128_t)c11 * c20) % q;
    uint128_t mul_c2 = ((uint128_t)c11 * c21) % q;
    
    cout << "Mul c0=" << (uint64_t)mul_c0 << " c1=" << (uint64_t)mul_c1 << " c2=" << (uint64_t)mul_c2 << "\n";
    
    uint128_t sum = (mul_c0 + mul_c1 + mul_c2) % q;
    uint128_t delta_sq = (uint128_t)delta * delta;
    cout << "Sum = " << (uint64_t)sum << "\n";
    cout << "Sum / delta² = " << (uint64_t)(sum / delta_sq) << " (exp " << (m1*m2) << ")\n";
    cout << "delta²*6 fits in uint64? delta²*6=" << (uint64_t)(6 * delta_sq) << "\n";
    
    return 0;
}
