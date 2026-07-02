#include "../src/core/true_poly_fhe.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  TRUE POLYNOMIAL FHE v6 — Full Range + Integrity" << endl;
    cout << "======================================================" << endl;

    TruePolyFHE fhe;
    uint64_t seed = 42;
    int pass = 0, total = 10;

    // 1. Enc/Dec
    auto ct = fhe.encrypt(42, seed);
    int64_t dec = fhe.decrypt(ct, seed);
    bool ok1 = (dec == 42);
    cout << "\n1. Enc/Dec: 42=" << dec << " " << (ok1 ? "✅" : "❌") << endl;
    if (ok1) pass++;

    // 2. Add
    auto a = fhe.encrypt(100, seed);
    auto b = fhe.encrypt(200, seed);
    auto c = fhe.add(a, b);
    int64_t dec2 = fhe.decrypt(c, seed);
    bool ok2 = (dec2 == 300);
    cout << "2. Poly Add: 100+200=" << dec2 << " " << (ok2 ? "✅" : "❌") << endl;
    if (ok2) pass++;

    // 3. Multiply
    auto x = fhe.encrypt(6, seed);
    auto y = fhe.encrypt(7, seed);
    auto z = fhe.multiply(x, y);
    int64_t dec3 = fhe.decrypt(z, seed);
    bool ok3 = (dec3 == 42);
    cout << "3. Poly Mul: 6×7=" << dec3 << " " << (ok3 ? "✅" : "❌") << endl;
    if (ok3) pass++;

    // 4. Chain of 10 adds
    auto acc = fhe.encrypt(0, seed);
    for (int i = 0; i < 10; i++) {
        auto one = fhe.encrypt(1, seed);
        acc = fhe.add(acc, one);
    }
    int64_t dec4 = fhe.decrypt(acc, seed);
    bool ok4 = (dec4 == 10);
    cout << "4. Chain (10 adds): " << dec4 << " " << (ok4 ? "✅" : "❌") << endl;
    if (ok4) pass++;

    // 5. Mixed: (3+4)×5
    auto a2 = fhe.encrypt(3, seed);
    auto b2 = fhe.encrypt(4, seed);
    auto c2 = fhe.encrypt(5, seed);
    auto sum = fhe.add(a2, b2);
    auto prod = fhe.multiply(sum, c2);
    int64_t dec5 = fhe.decrypt(prod, seed);
    bool ok5 = (dec5 == 35);
    cout << "5. Mixed (3+4)×5=" << dec5 << " " << (ok5 ? "✅" : "❌") << endl;
    if (ok5) pass++;

    // 6. Negative add
    auto ng = fhe.encrypt(-50, seed);
    auto ps = fhe.encrypt(100, seed);
    auto sum2 = fhe.add(ng, ps);
    int64_t dec6 = fhe.decrypt(sum2, seed);
    bool ok6 = (dec6 == 50);
    cout << "6. -50+100=" << dec6 << " " << (ok6 ? "✅" : "❌") << endl;
    if (ok6) pass++;

    // 7. IND-CPA
    auto ct1 = fhe.encrypt(99, seed);
    auto ct2 = fhe.encrypt(99, seed);
    bool ok7 = (ct1.nonce != ct2.nonce);
    cout << "7. IND-CPA (unique nonces): " << (ok7 ? "✅" : "❌") << endl;
    if (ok7) pass++;

    // 8. Noise bounded
    auto ct_a = fhe.encrypt(1, seed);
    auto ct_b = fhe.encrypt(1, seed);
    auto ct_sum = fhe.add(ct_a, ct_b);
    auto ct_prod = fhe.multiply(ct_a, ct_b);
    bool ok8 = (ct_prod.noise_level < 10.0);
    cout << "8. Noise: fresh=" << ct_a.noise_level 
         << " sum=" << ct_sum.noise_level 
         << " prod=" << ct_prod.noise_level;
    cout << " " << (ok8 ? "✅" : "❌") << endl;
    if (ok8) pass++;

    // 9. INT64_MAX (full range)
    auto ct_max = fhe.encrypt(INT64_MAX, seed);
    int64_t dec_max = fhe.decrypt(ct_max, seed);
    bool ok9 = (dec_max == INT64_MAX);
    cout << "9. INT64_MAX: " << (ok9 ? "✅" : "❌") << endl;
    if (!ok9) cout << "   got=" << dec_max << " expected=" << INT64_MAX << endl;
    if (ok9) pass++;

    // 10. Tampering DETECTED (should throw)
    auto ct_tamper = fhe.encrypt(42, seed);
    ct_tamper.coeffs[0] += 1000;  // Tamper!
    bool tamper_detected = false;
    try {
        fhe.decrypt(ct_tamper, seed);
    } catch (const std::runtime_error& e) {
        tamper_detected = true;
        cout << "10. Tamper detection: ✅ — " << e.what() << endl;
    }
    if (tamper_detected) pass++;

    cout << "\n======================================================" << endl;
    cout << "  TRUE POLY FHE: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    return (pass == total) ? 0 : 1;
}
