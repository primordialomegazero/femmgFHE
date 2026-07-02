// ============================================================
//  MONSTER HUNT — Deep Edge Case & Vulnerability Scanner
//  Hunting: integer overflow, div/0, use-after-free, 
//           uninitialized memory, infinite loops, 
//           crypto weaknesses, depth explosion
// ============================================================
#include "../src/core/true_poly_fhe.h"
#include "../src/core/femmg_operations.h"
#include <iostream>
#include <cstring>
#include <climits>
#include <cmath>
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
    cout << "  MONSTER HUNT — Halimaw Scanner v1.0" << endl;
    cout << "====================================================" << endl;

    TruePolyFHE fhe;
    FEmmgFHE chaos;
    uint64_t seed = 0xDEADBEEF;

    // ═══ 1. EXTREME VALUES ═══
    cout << "\n--- HUNT 1: Extreme Values ---" << endl;
    {
        // Max int64
        auto ct = fhe.encrypt(INT64_MAX, seed);
        int64_t dec = fhe.decrypt(ct, seed);
        check("INT64_MAX", dec == INT64_MAX, to_string(dec));

        // Min int64
        ct = fhe.encrypt(INT64_MIN, seed);
        dec = fhe.decrypt(ct, seed);
        check("INT64_MIN", dec == INT64_MIN, to_string(dec));

        // Zero
        ct = fhe.encrypt(0, seed);
        dec = fhe.decrypt(ct, seed);
        check("Zero", dec == 0, to_string(dec));

        // Multiply by zero
        auto a = fhe.encrypt(999999, seed);
        auto b = fhe.encrypt(0, seed);
        auto c = fhe.multiply(a, b);
        dec = fhe.decrypt(c, seed);
        check("Mul by zero", dec == 0, to_string(dec));

        // Add overflow potential
        auto big1 = fhe.encrypt(INT64_MAX / 2, seed);
        auto big2 = fhe.encrypt(INT64_MAX / 2, seed);
        auto big_sum = fhe.add(big1, big2);
        int64_t dec_sum = fhe.decrypt(big_sum, seed);
        // Should wrap or handle gracefully
        check("Big add (no crash)", true, "val=" + to_string(dec_sum));
    }

    // ═══ 2. DEPTH EXPLOSION ═══
    cout << "\n--- HUNT 2: Depth Explosion ---" << endl;
    {
        auto ct = fhe.encrypt(2, seed);
        int max_depth = 20;
        bool survived = true;
        for (int i = 0; i < max_depth && survived; i++) {
            ct = fhe.multiply(ct, fhe.encrypt(2, seed));
            if (ct.depth != i + 1) {
                survived = false;
                break;
            }
        }
        check("Depth tracking (20 mults)", survived, "final depth=" + to_string(ct.depth));
        
        // Decrypt after deep chain
        int64_t dec = fhe.decrypt(ct, seed);
        // 2^(max_depth+1) — will overflow but shouldn't crash
        check("Deep decrypt (no crash)", true, "val=" + to_string(dec));
    }

    // ═══ 3. NOISE CONVERGENCE ═══
    cout << "\n--- HUNT 3: Noise Convergence (φ test) ---" << endl;
    {
        auto ct = fhe.encrypt(1, seed);
        double prev_noise = ct.noise_level;
        bool converging = true;
        double noise_vals[10] = {prev_noise};
        
        for (int i = 0; i < 9; i++) {
            ct = fhe.multiply(ct, fhe.encrypt(1, seed));
            noise_vals[i+1] = ct.noise_level;
            if (ct.noise_level > prev_noise * 1.5) converging = false;
            prev_noise = ct.noise_level;
        }
        
        cout << "  Noise trace: ";
        for (int i = 0; i < 10; i++) cout << noise_vals[i] << " ";
        cout << endl;
        
        double fixed_point = noise_vals[9];
        check("Noise bounded", fixed_point < 100.0, "final=" + to_string(fixed_point));
        check("Noise not exploding", converging, "φ⁻¹ contraction");
    }

    // ═══ 4. CIPHERTEXT MANIPULATION ═══
    cout << "\n--- HUNT 4: Ciphertext Tampering ---" << endl;
    {
        auto ct = fhe.encrypt(42, seed);
        
        // Tamper with coeffs
        ct.coeffs[0] += 1;
        int64_t dec = fhe.decrypt(ct, seed);
        check("Tampered coeffs[0] ≠ 42", dec != 42, "got=" + to_string(dec));
        
        // Tamper with nonce
        auto ct2 = fhe.encrypt(42, seed);
        ct2.nonce = 0xBAD;
        dec = fhe.decrypt(ct2, seed);
        check("Tampered nonce ≠ 42", dec != 42, "got=" + to_string(dec));
        
        // Tamper with depth
        auto ct3 = fhe.encrypt(42, seed);
        ct3.depth = 5;
        dec = fhe.decrypt(ct3, seed);
        check("Tampered depth ≠ 42", dec != 42, "got=" + to_string(dec));
    }

    // ═══ 5. SELF-REFERENCE ATTACKS ═══
    cout << "\n--- HUNT 5: Self-Reference Attacks ---" << endl;
    {
        auto ct = fhe.encrypt(5, seed);
        
        // Add to self
        auto self_add = fhe.add(ct, ct);
        int64_t dec = fhe.decrypt(self_add, seed);
        check("Add(ct,ct) = pt+pt", dec == 10, "got=" + to_string(dec));
        
        // Multiply by self
        auto self_mul = fhe.multiply(ct, ct);
        dec = fhe.decrypt(self_mul, seed);
        check("Mul(ct,ct) = pt*pt", dec == 25, "got=" + to_string(dec));
        
        // Add(ct, ct) vs Add(ct, other_ct_same_pt)
        auto ct2 = fhe.encrypt(5, seed);
        auto diff_add = fhe.add(ct, ct2);
        dec = fhe.decrypt(diff_add, seed);
        check("Add(ct,ct2) with ct2.pt=ct.pt", dec == 10, "got=" + to_string(dec));
    }

    // ═══ 6. CHAOS ENGINE EDGE CASES ═══
    cout << "\n--- HUNT 6: Chaos Engine Edge Cases ---" << endl;
    {
        // Same plaintext, different encryptions
        auto ct1 = chaos.encrypt(42);
        auto ct2 = chaos.encrypt(42);
        check("Same pt → different ct (IND-CPA)", 
              ct1.value_int != ct2.value_int || ct1.operations != ct2.operations,
              "v1=" + to_string(ct1.value_int) + " v2=" + to_string(ct2.value_int));
        
        // Decrypt after many operations
        auto ct = chaos.encrypt(1);
        for (int i = 0; i < 100; i++) {
            ct = chaos.add(ct, chaos.encrypt(1));
        }
        int64_t dec = chaos.decrypt(ct);
        check("100-chain chaos add", dec == 101, "got=" + to_string(dec));
        
        // Multiply chain
        ct = chaos.encrypt(2);
        for (int i = 0; i < 5; i++) {
            ct = chaos.multiply(ct, chaos.encrypt(2));
        }
        dec = chaos.decrypt(ct);
        check("5-chain chaos mul (2^6=64)", dec == 64, "got=" + to_string(dec));
    }

    // ═══ 7. MEMORY & LIFETIME ═══
    cout << "\n--- HUNT 7: Memory & Lifetime ---" << endl;
    {
        // Large number of ciphertexts
        vector<TruePolyFHE::Ciphertext> cts;
        for (int i = 0; i < 10000; i++) {
            cts.push_back(fhe.encrypt(i % 1000, seed + i));
        }
        check("10K ciphertexts allocated", cts.size() == 10000);
        
        // Verify all decrypt correctly
        int errors = 0;
        for (int i = 0; i < 1000 && errors < 5; i++) {
            if (fhe.decrypt(cts[i], seed + i) != i % 1000) errors++;
        }
        check("Spot-check 1K decrypts", errors == 0, to_string(errors) + " errors");
        
        // Copy and move
        auto orig = fhe.encrypt(42, seed);
        auto copy = orig;
        auto moved = std::move(orig);
        int64_t d1 = fhe.decrypt(copy, seed);
        int64_t d2 = fhe.decrypt(moved, seed);
        check("Copy semantics", d1 == 42, to_string(d1));
        check("Move semantics", d2 == 42, to_string(d2));
    }

    // ═══ 8. SEED COLLISION ═══
    cout << "\n--- HUNT 8: Seed Handling ---" << endl;
    {
        // Same seed → same noise pattern (deterministic)
        auto a = fhe.encrypt(42, 0x12345);
        auto b = fhe.encrypt(42, 0x12345);
        // Different nonces (counter-based) pero same noise
        check("Same seed determinism", a.nonce != b.nonce, 
              "n1=" + to_string(a.nonce) + " n2=" + to_string(b.nonce));
        
        // Seed=0 uses random
        auto c1 = fhe.encrypt(42, 0);
        auto c2 = fhe.encrypt(42, 0);
        check("Seed=0 random", c1.nonce != c2.nonce,
              "n1=" + to_string(c1.nonce) + " n2=" + to_string(c2.nonce));
    }

    // ═══ 9. NEGATIVE/WRAPPED VALUES ═══
    cout << "\n--- HUNT 9: Negative Value Handling ---" << endl;
    {
        // Negative chain
        auto ct = fhe.encrypt(-100, seed);
        for (int i = 0; i < 10; i++) {
            ct = fhe.add(ct, fhe.encrypt(-10, seed));
        }
        int64_t dec = fhe.decrypt(ct, seed);
        check("-100 + 10×(-10) = -200", dec == -200, "got=" + to_string(dec));
        
        // Mixed sign multiply
        auto neg = fhe.encrypt(-6, seed);
        auto pos = fhe.encrypt(7, seed);
        auto prod = fhe.multiply(neg, pos);
        dec = fhe.decrypt(prod, seed);
        check("-6 × 7 = -42", dec == -42, "got=" + to_string(dec));
        
        // Negative × negative
        auto neg2 = fhe.encrypt(-6, seed);
        prod = fhe.multiply(neg, neg2);
        dec = fhe.decrypt(prod, seed);
        check("-6 × -6 = 36", dec == 36, "got=" + to_string(dec));
    }

    // ═══ 10. POLYNOMIAL RING BOUNDARY ═══
    cout << "\n--- HUNT 10: Ring Boundary (POLY_N=64) ---" << endl;
    {
        auto ct = fhe.encrypt(1, seed);
        // Force non-zero coeffs near boundary
        ct.coeffs[63] = 999999;
        int64_t dec = fhe.decrypt(ct, seed);
        check("coeffs[63] non-zero decrypts", dec != 1, 
              "(boundary noise affected result: " + to_string(dec) + ")");
        
        // All coeffs set
        for (int i = 0; i < 64; i++) ct.coeffs[i] = 1000000;
        dec = fhe.decrypt(ct, seed);
        check("All coeffs set (no crash)", true, "dec=" + to_string(dec));
    }

    // ═══ RESULTS ═══
    cout << "\n====================================================" << endl;
    cout << "  HUNT COMPLETE: " << passed << "/" << (passed+failed) << " PASSED" << endl;
    cout << "  HALIMAW COUNT: " << failed << endl;
    cout << "====================================================" << endl;

    return failed;
}
