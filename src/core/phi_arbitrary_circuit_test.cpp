// ΦΩ0 — ARBITRARY CIRCUIT TEST v2.0
// Earth OS Patch v2: Adaptive Multi-Round Divine + Extended stress tests
// "EVERY GATE. EVERY WIRE. VERIFIED. NO LIMITS."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include "phi_arbitrary_circuit.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — ARBITRARY CIRCUIT ENGINE v2.0: ADAPTIVE DIVINE   ║\n";
    cout <<   "  ║   Multi-Round Divine + Extended Stress Tests              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    int passed = 0, failed = 0;

    // ==========================================
    // TEST 1: (A × B) + C
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 1: (A × B) + C                                     │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        auto a = eng.add_input("A", 7);
        auto b = eng.add_input("B", 13);
        auto c = eng.add_input("C", 5);
        auto mul_ab = eng.add_mul("A×B", a, b);
        auto result = eng.add_add("(A×B)+C", mul_ab, c);
        eng.execute();
        int64_t expected = mod_pos(7*13+5, modulus);
        bool ok = (eng.get_plaintext(result) == expected);
        cout << "  TEST 1: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 2: (A×B) + (C×D) parallel
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 2: (A×B) + (C×D) parallel                          │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        auto a = eng.add_input("A", 3);
        auto b = eng.add_input("B", 11);
        auto c = eng.add_input("C", 7);
        auto d = eng.add_input("D", 2);
        auto b1 = eng.add_mul("A×B", a, b);
        auto b2 = eng.add_mul("C×D", c, d);
        auto result = eng.add_add("Sum", b1, b2);
        eng.execute();
        int64_t expected = mod_pos(3*11+7*2, modulus);
        bool ok = (eng.get_plaintext(result) == expected);
        cout << "  TEST 2: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 3: Complex DAG with fan-out
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 3: (A×B) + (C×4) - A (fan-out)                     │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        auto a = eng.add_input("A", 7);
        auto b = eng.add_input("B", 13);
        auto c = eng.add_input("C", 5);
        auto b1 = eng.add_mul("A×B", a, b);
        auto b2 = eng.add_mul_scalar("C×4", c, 4);
        auto sum_b = eng.add_add("B1+B2", b1, b2);
        auto result = eng.add_sub("Result-A", sum_b, a);
        eng.execute();
        int64_t expected = mod_pos(7*13+5*4-7, modulus);
        bool ok = (eng.get_plaintext(result) == expected);
        cout << "  TEST 3: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 4: DEEP CHAIN — 100 sequential ×3
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 4: DEEP CHAIN — 100 sequential ×3                   │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        int chain_len = 100;
        auto current = eng.add_input("start", 2);
        for (int i = 0; i < chain_len; i++) {
            current = eng.add_mul_scalar("×3_"+to_string(i+1), current, 3);
        }
        eng.execute();
        int64_t expected = 2;
        for (int i = 0; i < chain_len; i++) expected = mod_pos(expected * 3, modulus);
        bool ok = (eng.get_plaintext(current) == expected);
        cout << "  TEST 4: " << (ok ? "PASSED" : "FAILED") 
             << " (expected=" << expected << ")\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 5: WIDE FAN-IN — 25 parallel branches
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 5: WIDE FAN-IN — 25 parallel ×2 branches → sum      │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        vector<int> branches;
        for (int i = 0; i < 25; i++) {
            auto input = eng.add_input("in_"+to_string(i), 1);
            auto doubled = eng.add_mul_scalar("×2_"+to_string(i), input, 2);
            branches.push_back(doubled);
        }
        auto sum = branches[0];
        for (size_t i = 1; i < branches.size(); i++) {
            sum = eng.add_add("sum_"+to_string(i), sum, branches[i]);
        }
        eng.execute();
        bool ok = (eng.get_plaintext(sum) == 50);
        cout << "  TEST 5: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 6: MIXED DEEP + WIDE — 10 chains of 10
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 6: MIXED — 10 chains ×10 deep, then sum             │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        vector<int> chain_outputs;
        for (int c = 0; c < 10; c++) {
            auto val = eng.add_input("c"+to_string(c)+"_start", c+1);
            for (int s = 0; s < 10; s++) {
                val = eng.add_mul_scalar("c"+to_string(c)+"_×2_"+to_string(s), val, 2);
            }
            chain_outputs.push_back(val);
        }
        auto total = chain_outputs[0];
        for (size_t i = 1; i < chain_outputs.size(); i++) {
            total = eng.add_add("total_"+to_string(i), total, chain_outputs[i]);
        }
        eng.execute();
        // Expected: sum((c+1) * 2^10) = 1024 * sum(1..10) = 1024 * 55 = 56320
        int64_t expected = 0;
        for (int c = 0; c < 10; c++) {
            int64_t v = c+1;
            for (int s = 0; s < 10; s++) v = mod_pos(v * 2, modulus);
            expected = mod_pos(expected + v, modulus);
        }
        bool ok = (eng.get_plaintext(total) == expected);
        cout << "  TEST 6: " << (ok ? "PASSED" : "FAILED") 
             << " (expected=" << expected << ")\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 7: ULTRA DEEP — 250 sequential ×2
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 7: ULTRA DEEP — 250 sequential ×2                   │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        ArbitraryCircuitEngine eng(cc, keys, modulus);
        int chain_len = 250;
        auto current = eng.add_input("start", 1);
        for (int i = 0; i < chain_len; i++) {
            current = eng.add_mul_scalar("×2_"+to_string(i+1), current, 2);
        }
        eng.execute();
        int64_t expected = 1;
        for (int i = 0; i < chain_len; i++) expected = mod_pos(expected * 2, modulus);
        bool ok = (eng.get_plaintext(current) == expected);
        cout << "  TEST 7: " << (ok ? "PASSED" : "FAILED") 
             << " (expected=" << expected << ")\n";
        if (ok) passed++; else failed++;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS: " << passed << " PASSED, " << failed << " FAILED out of " << (passed+failed) << " tests           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
