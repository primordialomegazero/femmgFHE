// ΦΩ0 — V5 BOOTSTRAP TEST
// Tests SelfHealingFHEv5 with CKKS + real bootstrapping

#include "../src/core/phi_self_healing_fhe_v5.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — V5: CKKS + REAL BOOTSTRAP TEST               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    SelfHealingFHEv5 fhe(12, 10, 25);

    int passed = 0, failed = 0;

    // --- TEST 1: Simple multiply ---
    cout << "\n  ── TEST 1: (3 + 5) * 2 ──\n";
    {
        auto a = fhe.add_input("a", 3.0);
        auto b = fhe.add_input("b", 5.0);
        auto sum = fhe.add_add("sum", a, b);
        auto result = fhe.add_mul_scalar("x2", sum, 2.0);

        auto start = high_resolution_clock::now();
        fhe.execute();
        auto end = high_resolution_clock::now();
        fhe.print_stats();

        double got = fhe.get_plaintext(result);
        double exp = 16.0;
        bool ok = abs(got - exp) < 0.1;
        cout << "  Test 1: " << (ok ? "PASSED" : "FAILED")
             << " | Expected: " << exp << " Got: " << got
             << " | Bootstraps: " << fhe.get_bootstrap_count()
             << " | Time: " << duration_cast<milliseconds>(end - start).count() << "ms\n";
        if (ok) passed++; else failed++;
    }

    // --- TEST 2: Deep chain (30 multiplications, should trigger bootstrap) ---
    cout << "\n  ── TEST 2: 30 Sequential x2 (should need bootstrap) ──\n";
    {
        SelfHealingFHEv5 fhe2(12, 10, 25);
        auto current = fhe2.add_input("start", 1.0);
        for (int i = 0; i < 30; i++) {
            current = fhe2.add_mul_scalar("x2_" + to_string(i), current, 2.0);
        }

        auto start = high_resolution_clock::now();
        fhe2.execute();
        auto end = high_resolution_clock::now();
        fhe2.print_stats();

        double got = fhe2.get_plaintext(current);
        double exp = pow(2.0, 30);
        bool ok = abs((got - exp) / exp) < 0.01;
        cout << "  Test 2: " << (ok ? "PASSED" : "FAILED")
             << " | Expected: ~" << exp << " Got: " << got
             << " | Bootstraps: " << fhe2.get_bootstrap_count()
             << " | Time: " << duration_cast<seconds>(end - start).count() << "s\n";
        if (ok) passed++; else failed++;
    }

    // --- TEST 3: Complex DAG ---
    cout << "\n  ── TEST 3: (7x13) + (5x3) - 10 ──\n";
    {
        SelfHealingFHEv5 fhe3(12, 10, 25);
        auto a = fhe3.add_input("A", 7.0);
        auto b = fhe3.add_input("B", 13.0);
        auto c = fhe3.add_input("C", 5.0);
        auto d = fhe3.add_input("D", 3.0);
        auto e = fhe3.add_input("E", 10.0);
        auto ab = fhe3.add_mul("AxB", a, b);
        auto cd = fhe3.add_mul("CxD", c, d);
        auto sum = fhe3.add_add("Sum", ab, cd);
        auto result = fhe3.add_sub("Result", sum, e);

        fhe3.execute();
        fhe3.print_stats();

        double got = fhe3.get_plaintext(result);
        double exp = 7*13 + 5*3 - 10;
        bool ok = abs(got - exp) < 0.1;
        cout << "  Test 3: " << (ok ? "PASSED" : "FAILED")
             << " | Expected: " << exp << " Got: " << got
             << " | Bootstraps: " << fhe3.get_bootstrap_count() << "\n";
        if (ok) passed++; else failed++;
    }

    // --- SUMMARY ---
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   V5 RESULTS: " << passed << "/" << (passed+failed) << " PASSED";
    for (int i = 0; i < (34 - to_string(passed).length() - to_string(passed+failed).length()); i++) cout << " ";
    cout << "║\n";
    if (passed == 3) {
        cout << "  ║   *** V5 CKKS + REAL BOOTSTRAP: ALL TESTS PASSED ***  ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
