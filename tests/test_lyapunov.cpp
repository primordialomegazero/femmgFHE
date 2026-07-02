#include "../src/core/lyapunov_fhe.h"
#include <iostream>
#include <iomanip>
#include <climits>
#include <cmath>
using namespace std;

int main() {
    cout << "============================================================" << endl;
    cout << "  LYAPUNOV-STABILIZED FLOATING POINT FHE" << endl;
    cout << "  Mantissa: 53-bit | Exponent: ±1023 | Range: ±10^±308" << endl;
    cout << "  Noise: φ⁻¹ Banach contraction → fixed point 1.828" << endl;
    cout << "============================================================" << endl;
    
    LyapunovFHE fhe;
    uint64_t seed = 42;
    int pass = 0, total = 12;

    // 1. Basic double encrypt/decrypt
    auto ct1 = fhe.encrypt(3.141592653589793, seed);
    double dec1 = fhe.decrypt(ct1, seed);
    bool ok1 = (abs(dec1 - 3.141592653589793) < 1e-10);
    cout << "\n1. Enc/Dec π: " << setprecision(15) << dec1 << (ok1 ? " ✅" : " ❌") << endl;
    if (ok1) pass++;

    // 2. Integer encrypt/decrypt
    auto ct2 = fhe.encrypt((int64_t)42, seed);
    double dec2 = fhe.decrypt(ct2, seed);
    bool ok2 = (dec2 == 42.0);
    cout << "2. Enc/Dec 42: " << dec2 << (ok2 ? " ✅" : " ❌") << endl;
    if (ok2) pass++;

    // 3. INT64_MAX
    auto ct3 = fhe.encrypt((double)INT64_MAX, seed);
    double dec3 = fhe.decrypt(ct3, seed);
    double err3 = abs(dec3 - (double)INT64_MAX) / (double)INT64_MAX;
    bool ok3 = (err3 < 1e-12);
    cout << "3. INT64_MAX: " << setprecision(20) << dec3 << " (rel err: " << err3 << ")" 
         << (ok3 ? " ✅" : " ❌") << endl;
    if (ok3) pass++;

    // 4. INT64_MIN
    auto ct4 = fhe.encrypt((double)INT64_MIN, seed);
    double dec4 = fhe.decrypt(ct4, seed);
    double err4 = abs(dec4 - (double)INT64_MIN) / abs((double)INT64_MIN);
    bool ok4 = (err4 < 1e-12);
    cout << "4. INT64_MIN: " << setprecision(20) << dec4 << " (rel err: " << err4 << ")" 
         << (ok4 ? " ✅" : " ❌") << endl;
    if (ok4) pass++;

    // 5. Floating add
    auto a = fhe.encrypt(100.5, seed);
    auto b = fhe.encrypt(200.25, seed);
    auto c = fhe.add(a, b);
    double dec5 = fhe.decrypt(c, seed);
    bool ok5 = (abs(dec5 - 300.75) < 1e-10);
    cout << "5. Blind Add: 100.5+200.25=" << dec5 << (ok5 ? " ✅" : " ❌") << endl;
    if (ok5) pass++;

    // 6. Floating multiply
    auto x = fhe.encrypt(6.5, seed);
    auto y = fhe.encrypt(7.25, seed);
    auto z = fhe.multiply(x, y);
    double dec6 = fhe.decrypt(z, seed);
    bool ok6 = (abs(dec6 - 47.125) < 1e-9);
    cout << "6. Blind Mul: 6.5×7.25=" << dec6 << (ok6 ? " ✅" : " ❌") << endl;
    if (ok6) pass++;

    // 7. Chain of adds (Lyapunov-stabilized)
    auto acc = fhe.encrypt(0.0, seed);
    for (int i = 0; i < 100; i++) {
        auto one = fhe.encrypt(1.0, seed);
        acc = fhe.add(acc, one);
    }
    double dec7 = fhe.decrypt(acc, seed);
    bool ok7 = (abs(dec7 - 100.0) < 1e-8);
    cout << "7. Chain (100 adds): " << dec7 << (ok7 ? " ✅" : " ❌") << endl;
    if (ok7) pass++;

    // 8. Mixed ops
    auto a2 = fhe.encrypt(3.0, seed);
    auto b2 = fhe.encrypt(4.0, seed);
    auto c2 = fhe.encrypt(5.0, seed);
    auto sum = fhe.add(a2, b2);
    auto prod = fhe.multiply(sum, c2);
    double dec8 = fhe.decrypt(prod, seed);
    bool ok8 = (abs(dec8 - 35.0) < 1e-9);
    cout << "8. Mixed (3+4)×5=" << dec8 << (ok8 ? " ✅" : " ❌") << endl;
    if (ok8) pass++;

    // 9. Negative values
    auto neg = fhe.encrypt(-50.0, seed);
    auto pos = fhe.encrypt(100.0, seed);
    auto sum2 = fhe.add(neg, pos);
    double dec9 = fhe.decrypt(sum2, seed);
    bool ok9 = (abs(dec9 - 50.0) < 1e-10);
    cout << "9. -50+100=" << dec9 << (ok9 ? " ✅" : " ❌") << endl;
    if (ok9) pass++;

    // 10. Multiply negatives
    auto ng1 = fhe.encrypt(-6.0, seed);
    auto ng2 = fhe.encrypt(-6.0, seed);
    auto ng_prod = fhe.multiply(ng1, ng2);
    double dec10 = fhe.decrypt(ng_prod, seed);
    bool ok10 = (abs(dec10 - 36.0) < 1e-9);
    cout << "10. -6×-6=" << dec10 << (ok10 ? " ✅" : " ❌") << endl;
    if (ok10) pass++;

    // 11. Lyapunov stability verification
    auto ct_stable = fhe.encrypt(1.0, seed);
    for (int i = 0; i < 10; i++) {
        ct_stable = fhe.multiply(ct_stable, fhe.encrypt(1.0, seed));
    }
    bool ok11 = fhe.verify_stability(ct_stable);
    cout << "11. Lyapunov stable after 10 mults: noise=" << ct_stable.noise_level 
         << " bound=" << ct_stable.lyapunov_bound 
         << (ok11 ? " ✅" : " ❌") << endl;
    if (ok11) pass++;

    // 12. Tampering detection
    auto ct_tamper = fhe.encrypt(42.0, seed);
    ct_tamper.coeffs[0] += 1000;
    bool tamper_detected = false;
    try {
        fhe.decrypt(ct_tamper, seed);
    } catch (const runtime_error& e) {
        tamper_detected = true;
        cout << "12. Tamper detection: ✅ — " << e.what() << endl;
    }
    if (tamper_detected) pass++;

    cout << "\n============================================================" << endl;
    cout << "  LYAPUNOV FHE: " << pass << "/" << total << " PASSED" << endl;
    cout << "============================================================" << endl;
    return (pass == total) ? 0 : 1;
}
