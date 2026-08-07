#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <cstdint>

constexpr double PHI = 1.6180339887498948482;

int64_t void_factor_dual(int64_t N, int64_t* steps_out = nullptr) {
    double sq = sqrt(N);
    
    // Two starting points:
    // Point A: sqrt(N)/φ — for wide prime pairs (q/p ≈ φ)
    // Point B: sqrt(N)   — for close prime pairs (q/p ≈ 1)
    int64_t start_wide = (int64_t)(sq / PHI);
    int64_t start_close = (int64_t)(sq);
    
    if (start_wide < 2) start_wide = 2;
    if (start_close >= N) start_close = N - 1;
    
    int64_t steps = 0;
    int64_t max_steps = (int64_t)(sq * 0.1); // 10% of sqrt(N) — enough for either direction
    if (max_steps > 100000000) max_steps = 100000000;
    
    // Walk inward from both sides toward the middle
    // The factor must be between start_wide and start_close
    int64_t lo = start_wide;
    int64_t hi = start_close;
    if (lo > hi) { int64_t tmp = lo; lo = hi; hi = tmp; }
    
    // Bidirectional search from both ends
    for (int64_t delta = 0; delta <= max_steps; delta++) {
        steps++;
        
        // From wide end (sqrt(N)/φ) moving up
        int64_t from_wide = lo + delta;
        if (from_wide >= 2 && from_wide < N && N % from_wide == 0) {
            if (steps_out) *steps_out = steps;
            return from_wide;
        }
        
        // From close end (sqrt(N)) moving down
        int64_t from_close = hi - delta;
        if (from_close >= 2 && from_close < N && from_close != from_wide && N % from_close == 0) {
            if (steps_out) *steps_out = steps;
            return from_close;
        }
    }
    
    if (steps_out) *steps_out = steps;
    return 0;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  VOID COLLAPSE DUAL-DIRECTION\n";
    std::cout << "  Wide end: sqrt(N)/φ  |  Close end: sqrt(N)\n";
    std::cout << "================================================================================\n\n";

    struct TestCase {
        int64_t N;
        std::string label;
    };

    TestCase tests[] = {
        // Wide prime pairs (q/p ≈ φ)
        {15, "3×5 (wide)"},
        {35, "5×7 (wide)"},
        {77, "7×11 (wide)"},
        {143, "11×13 (wide)"},
        {323, "17×19 (wide)"},
        {667, "23×29 (wide)"},
        {9797, "97×101 (wide)"},
        
        // Close prime pairs (q/p ≈ 1 — RSA style)
        {1000000016000000063LL, "RSA-100 style (close)"},
        
        // Medium-ratio pairs
        {221, "13×17 (medium)"},
        {437, "19×23 (medium)"},
        {899, "29×31 (medium)"},
        {1517, "37×41 (medium)"},
        {22499, "149×151 (close-ish)"},
        {32399, "179×181 (close-ish)"},
    };

    int success = 0;
    int total = sizeof(tests)/sizeof(tests[0]);

    for (auto& t : tests) {
        double sq = sqrt((long double)t.N);
        double wide_start = sq / PHI;
        double close_start = sq;
        
        auto t1 = std::chrono::steady_clock::now();
        int64_t steps = 0;
        int64_t p = void_factor_dual(t.N, &steps);
        auto t2 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        
        bool ok = (p > 0 && t.N % p == 0);
        if (ok) success++;
        
        std::cout << std::left << std::setw(30) << t.label
                  << " | wide=" << std::fixed << std::setprecision(0) << std::setw(12) << wide_start
                  << " | close=" << std::setw(12) << close_start
                  << " | found=" << std::setw(12) << p
                  << " | steps=" << std::setw(8) << steps
                  << " | " << (ok ? "OK" : "?") << "\n";
    }

    std::cout << "\n  Success: " << success << "/" << total << "\n";
    std::cout << "\n================================================================================\n\n";
}
