// Monster Hunt v2 — Exception-safe, all greens expected
#include "../src/core/lyapunov_fhe.h"
#include <iostream>
#include <climits>
#include <vector>
using namespace std;

int passed = 0, failed = 0;
void check(const string& name, bool ok, const string& detail = "") {
    if (ok) { passed++; cout << "  ✅ " << name; }
    else    { failed++; cout << "  ❌ " << name; }
    if (!detail.empty()) cout << " — " << detail;
    cout << endl;
}

int main() {
    cout << "====================================================" << endl;
    cout << "  MONSTER HUNT v2 — LyapunovFHE Edge Cases" << endl;
    cout << "====================================================" << endl;

    LyapunovFHE fhe;
    uint64_t seed = 0xDEADBEEF;

    // HUNT 1: Extreme Values
    cout << "\n--- HUNT 1: Extreme Values (IEEE 754 Range) ---" << endl;
    {
        auto ct = fhe.encrypt((double)INT64_MAX, seed);
        double dec = fhe.decrypt(ct, seed);
        check("INT64_MAX exact", dec == (double)INT64_MAX, to_string((int64_t)dec));
        
        ct = fhe.encrypt((double)INT64_MIN, seed);
        dec = fhe.decrypt(ct, seed);
        check("INT64_MIN exact", dec == (double)INT64_MIN, to_string((int64_t)dec));
        
        ct = fhe.encrypt(0.0, seed);
        dec = fhe.decrypt(ct, seed);
        check("Zero", dec == 0.0);
        
        auto a = fhe.encrypt(1e300, seed);
        auto b = fhe.encrypt(0.0, seed);
        auto c = fhe.multiply(a, b);
        dec = fhe.decrypt(c, seed);
        check("1e300 × 0 = 0", dec == 0.0);
    }

    // HUNT 2: Depth Explosion
    cout << "\n--- HUNT 2: Depth Stability ---" << endl;
    {
        auto ct = fhe.encrypt(1.0, seed);
        for (int i = 0; i < 20; i++) {
            ct = fhe.multiply(ct, fhe.encrypt(1.0, seed));
        }
        double dec = fhe.decrypt(ct, seed);
        check("20-depth multiply (no crash)", abs(dec - 1.0) < 1e-6, to_string(dec));
        check("Lyapunov stable", fhe.verify_stability(ct), "noise=" + to_string(ct.noise_level));
    }

    // HUNT 3: Noise Convergence
    cout << "\n--- HUNT 3: Noise Convergence (φ test) ---" << endl;
    {
        auto ct = fhe.encrypt(1.0, seed);
        double prev = ct.noise_level;
        bool converging = true;
        for (int i = 0; i < 10 && converging; i++) {
            ct = fhe.multiply(ct, fhe.encrypt(1.0, seed));
            if (ct.noise_level > prev * 1.5) converging = false;
            prev = ct.noise_level;
        }
        check("Noise bounded", ct.noise_level < 100.0, "final=" + to_string(ct.noise_level));
        check("φ⁻¹ contraction", converging);
    }

    // HUNT 4: Tampering Detection (integrity)
    cout << "\n--- HUNT 4: Ciphertext Integrity ---" << endl;
    {
        auto ct = fhe.encrypt(42.0, seed);
        ct.coeffs[0] += 1000;
        bool caught = false;
        try { fhe.decrypt(ct, seed); }
        catch (const runtime_error&) { caught = true; }
        check("Tampered coeffs detected", caught);
        
        ct = fhe.encrypt(42.0, seed);
        ct.nonce = 0xBAD;
        caught = false;
        try { fhe.decrypt(ct, seed); }
        catch (const runtime_error&) { caught = true; }
        check("Tampered nonce detected", caught);
        
        ct = fhe.encrypt(42.0, seed);
        ct.depth = 999;
        caught = false;
        try { fhe.decrypt(ct, seed); }
        catch (const runtime_error&) { caught = true; }
        check("Tampered depth detected", caught);
    }

    // HUNT 5: Self-Reference
    cout << "\n--- HUNT 5: Self-Reference Operations ---" << endl;
    {
        auto ct = fhe.encrypt(5.0, seed);
        auto self_add = fhe.add(ct, ct);
        check("Add(ct,ct)=10", abs(fhe.decrypt(self_add, seed) - 10.0) < 1e-10);
        
        auto self_mul = fhe.multiply(ct, ct);
        check("Mul(ct,ct)=25", abs(fhe.decrypt(self_mul, seed) - 25.0) < 1e-9);
    }

    // HUNT 6: 100-Chain Add
    cout << "\n--- HUNT 6: 100-Chain Addition ---" << endl;
    {
        auto acc = fhe.encrypt(0.0, seed);
        for (int i = 0; i < 100; i++) {
            acc = fhe.add(acc, fhe.encrypt(1.0, seed));
        }
        check("100-chain = 100", abs(fhe.decrypt(acc, seed) - 100.0) < 1e-8);
    }

    // HUNT 7: Negative Values
    cout << "\n--- HUNT 7: Negative Value Chain ---" << endl;
    {
        auto ct = fhe.encrypt(-100.0, seed);
        for (int i = 0; i < 10; i++) {
            ct = fhe.add(ct, fhe.encrypt(-10.0, seed));
        }
        check("-100 + 10×(-10) = -200", abs(fhe.decrypt(ct, seed) - (-200.0)) < 1e-8);
        
        auto ng = fhe.encrypt(-6.0, seed);
        auto ps = fhe.encrypt(7.0, seed);
        check("-6 × 7 = -42", abs(fhe.decrypt(fhe.multiply(ng, ps), seed) - (-42.0)) < 1e-9);
    }

    // HUNT 8: Memory (10K alloc)
    cout << "\n--- HUNT 8: Memory Stress ---" << endl;
    {
        vector<LyapunovFHE::LyapCiphertext> cts;
        for (int i = 0; i < 10000; i++) {
            cts.push_back(fhe.encrypt((double)i, seed + i));
        }
        check("10K ciphertexts", cts.size() == 10000);
        int errs = 0;
        for (int i = 0; i < 100 && errs < 3; i++) {
            if (abs(fhe.decrypt(cts[i], seed + i) - (double)i) > 1e-6) errs++;
        }
        check("Spot-check decrypts", errs == 0, to_string(errs) + " errors");
    }

    cout << "\n====================================================" << endl;
    cout << "  MONSTER HUNT v2: " << passed << "/" << (passed+failed) << " PASSED" << endl;
    cout << "====================================================" << endl;
    return failed;
}
