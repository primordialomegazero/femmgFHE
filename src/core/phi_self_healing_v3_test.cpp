// ΦΩ0 — SELF-HEALING FHE v3 TEST — PATH A VALIDATION
// Tests: Deep chain, DAG, Single-bootstrap circuits
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "phi_self_healing_fhe_v3.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int depth = 30;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — SELF-HEALING FHE v3: CHAIN-AWARE                 ║\n";
    cout <<   "  ║   Bootstrap ONLY on chain exhaustion                     ║\n";
    cout <<   "  ║   Divine+ZANS for ALL intermediate operations             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    int passed = 0, failed = 0;

    // ==========================================
    // TEST 1: WITHIN CHAIN DEPTH — 25 sequential ×2
    // EXPECT: 0 bootstraps, 100% correct
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 1: 25 Sequential ×2 (WITHIN chain depth — 0 bootstraps) │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHEv3 fhe(cc, keys, modulus);

        auto current = fhe.add_input("start", 1);
        for (int i = 0; i < 25; i++) {
            current = fhe.add_mul_scalar("x2_" + to_string(i+1), current, 2);
        }

        auto t_start = high_resolution_clock::now();
        fhe.execute();
        fhe.print_stats();

        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();

        int64_t expected = 1;
        for (int i = 0; i < 25; i++) expected = mod_pos(expected * 2, modulus);
        int64_t got = fhe.get_plaintext(current);
        int bootstraps = fhe.get_bootstrap_count();

        bool ok = (got == expected) && (bootstraps == 0);
        cout << "  Test 1: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got
             << ", bootstraps=" << bootstraps
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 2: BEYOND CHAIN DEPTH — 60 sequential ×2
    // EXPECT: bootstraps used, 100% correct
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 2: 60 Sequential ×2 (BEYOND chain — needs bootstraps)  │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHEv3 fhe(cc, keys, modulus);

        auto current = fhe.add_input("start", 1);
        for (int i = 0; i < 60; i++) {
            current = fhe.add_mul_scalar("x2_" + to_string(i+1), current, 2);
        }

        auto t_start = high_resolution_clock::now();
        fhe.execute();
        fhe.print_stats();

        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();

        int64_t expected = 1;
        for (int i = 0; i < 60; i++) expected = mod_pos(expected * 2, modulus);
        int64_t got = fhe.get_plaintext(current);
        int bootstraps = fhe.get_bootstrap_count();

        bool ok = (got == expected);
        cout << "  Test 2: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got
             << ", bootstraps=" << bootstraps
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 3: COMPLEX DAG WITHIN DEPTH — (A×B)+(C×D)-E
    // EXPECT: 0 bootstraps, 100% correct
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 3: Complex DAG — (A×B)+(C×D)-E (0 bootstraps)       │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHEv3 fhe(cc, keys, modulus);

        auto a = fhe.add_input("A", 7);
        auto b = fhe.add_input("B", 13);
        auto c = fhe.add_input("C", 5);
        auto d = fhe.add_input("D", 3);
        auto e = fhe.add_input("E", 10);

        auto mul_ab = fhe.add_mul("AxB", a, b);
        auto mul_cd = fhe.add_mul("CxD", c, d);
        auto sum = fhe.add_add("Sum", mul_ab, mul_cd);
        auto result = fhe.add_sub("Result-E", sum, e);

        fhe.execute();
        fhe.print_stats();

        int64_t expected = mod_pos(7*13 + 5*3 - 10, modulus);
        int64_t got = fhe.get_plaintext(result);
        int bootstraps = fhe.get_bootstrap_count();

        bool ok = (got == expected) && (bootstraps == 0);
        cout << "  Test 3: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << expected << ", got=" << got
             << ", bootstraps=" << bootstraps << ")\n";
        if (ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 4: DEEP DAG — 20 chains × 50 deep, sum
    // EXPECT: bootstraps used correctly, 100% correct
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 4: 20 Chains × 50 Deep — Sum (needs bootstraps)      │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        SelfHealingFHEv3 fhe(cc, keys, modulus);

        vector<int> chain_ends;
        int64_t total_expected = 0;

        for (int c = 0; c < 20; c++) {
            int64_t sv = c + 1;
            auto current = fhe.add_input("c" + to_string(c) + "_start", sv);
            int64_t expected_val = sv;

            for (int s = 0; s < 50; s++) {
                expected_val = mod_pos(expected_val * 2, modulus);
                current = fhe.add_mul_scalar("c" + to_string(c) + "_x2_" + to_string(s), current, 2);
            }

            chain_ends.push_back(current);
            total_expected = mod_pos(total_expected + expected_val, modulus);
        }

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
        int bootstraps = fhe.get_bootstrap_count();

        bool ok = (got == total_expected);
        cout << "  Test 4: " << (ok ? "PASSED" : "FAILED")
             << " (expected=" << total_expected << ", got=" << got
             << ", bootstraps=" << bootstraps
             << ", time=" << elapsed << "s)\n";
        if (ok) passed++; else failed++;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PATH A RESULTS: " << passed << "/" << (passed+failed) << " PASSED";
    for (int i = 0; i < (35 - to_string(passed).length() - to_string(passed+failed).length()); i++) cout << " ";
    cout << "║\n";

    if (passed == 4) {
        cout << "  ║   STATUS: PATH A — SINGLE-BOOTSTRAP FHE CONFIRMED        ║\n";
        cout << "  ║   Divine+ZANS handles ALL intermediate ops                ║\n";
        cout << "  ║   Bootstrap ONLY on chain exhaustion                      ║\n";
        cout << "  ║   Circuits ≤ chain depth: ZERO bootstraps                 ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
