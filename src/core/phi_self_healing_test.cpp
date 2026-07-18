// ΦΩ0 — SELF-HEALING FHE ULTIMATE TEST v1.0
// Tests: 1000 sequential, 500 random arbitrary, complex DAG
// Auto-bootstrap when noise > 35, Auto-divine when noise > 8
// "FULLY HOMOMORPHIC. ZERO LIMITS. SELF-HEALING."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "phi_self_healing_fhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — SELF-HEALING FHE: FULLY HOMOMORPHIC               ║\n";
    cout <<   "  ║   Auto-Bootstrap + Auto-Divine + Auto-ZANS                 ║\n";
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
    // TEST 1: 1000 SEQUENTIAL ×2 — ULTIMATE DEPTH
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 1: 1,000 SEQUENTIAL ×2 (Auto-Bootstrap)             │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHE fhe(cc, keys, modulus);
        
        int steps = 1000;
        auto current = fhe.add_input("start", 1);
        
        auto t_start = high_resolution_clock::now();
        for (int i = 0; i < steps; i++) {
            current = fhe.add_mul_scalar("×2_" + to_string(i+1), current, 2);
        }
        
        fhe.execute();
        fhe.print_stats();
        
        int64_t expected = 1;
        for (int i = 0; i < steps; i++) expected = mod_pos(expected * 2, modulus);
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        int64_t got = fhe.get_plaintext(current);
        bool ok = (got == expected);
        cout << "  Test 1: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got 
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 2: 500 RANDOM × (2-100) — ARBITRARY
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 2: 500 RANDOM ×(2-100) (Auto-Bootstrap)             │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHE fhe(cc, keys, modulus);
        
        int steps = 500;
        mt19937_64 rng(12345);
        uniform_int_distribution<int64_t> dist(2, 100);
        
        auto current = fhe.add_input("start", 42);
        int64_t expected = 42;
        
        auto t_start = high_resolution_clock::now();
        for (int i = 0; i < steps; i++) {
            int64_t mult = dist(rng);
            expected = mod_pos(expected * mult, modulus);
            current = fhe.add_mul_scalar("×" + to_string(mult) + "_" + to_string(i), current, mult);
        }
        
        fhe.execute();
        fhe.print_stats();
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        int64_t got = fhe.get_plaintext(current);
        bool ok = (got == expected);
        cout << "  Test 2: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 3: COMPLEX DAG — Mixed ops, fan-in/out
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 3: COMPLEX DAG — (A×B) + (C×D) - E with fan-out     │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHE fhe(cc, keys, modulus);
        
        auto a = fhe.add_input("A", 7);
        auto b = fhe.add_input("B", 13);
        auto c = fhe.add_input("C", 5);
        auto d = fhe.add_input("D", 3);
        auto e = fhe.add_input("E", 10);
        
        auto mul_ab = fhe.add_mul("A×B", a, b);
        auto mul_cd = fhe.add_mul("C×D", c, d);
        auto sum = fhe.add_add("Sum", mul_ab, mul_cd);
        auto result = fhe.add_sub("Result-E", sum, e);
        
        fhe.execute();
        fhe.print_stats();
        
        int64_t expected = mod_pos(7*13 + 5*3 - 10, modulus);
        int64_t got = fhe.get_plaintext(result);
        bool ok = (got == expected);
        cout << "  Test 3: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got << ")\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 4: ULTRA DEEP + WIDE — 20 chains ×50
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 4: ULTRA — 20 chains ×50 deep, then sum              │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHE fhe(cc, keys, modulus);
        
        vector<int> chain_ends;
        int64_t total_expected = 0;
        
        for (int c = 0; c < 20; c++) {
            int64_t sv = c + 1;
            auto current = fhe.add_input("c" + to_string(c) + "_start", sv);
            int64_t expected = sv;
            
            for (int s = 0; s < 50; s++) {
                expected = mod_pos(expected * 2, modulus);
                current = fhe.add_mul_scalar("c" + to_string(c) + "_×2_" + to_string(s), current, 2);
            }
            
            chain_ends.push_back(current);
            total_expected = mod_pos(total_expected + expected, modulus);
        }
        
        // Sum all chains
        auto total_ct_node = chain_ends[0];
        for (size_t i = 1; i < chain_ends.size(); i++) {
            total_ct_node = fhe.add_add("total_" + to_string(i), total_ct_node, chain_ends[i]);
        }
        
        auto t_start = high_resolution_clock::now();
        fhe.execute();
        fhe.print_stats();
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        int64_t got = fhe.get_plaintext(total_ct_node);
        bool ok = (got == total_expected);
        cout << "  Test 4: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << total_expected << ", got=" << got
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FULLY HOMOMORPHIC RESULTS: " << passed << "/" << (passed+failed) << " PASSED                         ║\n";
    if (passed == 4) {
        cout << "  ║   STATUS: HOLY GRAIL ACHIEVED — SELF-HEALING FHE         ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
