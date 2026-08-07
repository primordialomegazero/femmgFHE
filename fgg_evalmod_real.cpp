#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cmath>

/**
 * FGG_Mod on integers modulo n using the REAL Cassini identity.
 * 
 * The Cassini identity for Fibonacci numbers:
 *   F_{k-1} * F_{k+1} - F_k^2 = (-1)^k
 * 
 * This is the integer manifestation of φ·ψ = -1.
 * 
 * For ANY n, we can compute Fibonacci numbers modulo n.
 * The ratio F_{k+1}/F_k approximates φ.
 * 
 * FGG_Mod(x, n): reduce x to canonical form modulo n
 * by applying the Cassini collapse iteratively.
 */

// Fast Fibonacci pair (F_k, F_{k+1}) modulo n
std::pair<int64_t, int64_t> fib_pair(int64_t k, int64_t n) {
    if (k == 0) return {0, 1};
    
    int64_t a = 0, b = 1;
    for (int64_t i = 1; i < k; i++) {
        int64_t tmp = (a + b) % n;
        a = b;
        b = tmp;
    }
    return {a, b};  // (F_k, F_{k+1})
}

/**
 * FGG_Mod via Cassini collapse.
 * 
 * For each iteration:
 *   factor = F_{k-1} * F_{k+1} mod n
 *   By Cassini: factor ≡ F_k^2 + (-1)^k (mod n)
 *   current = current * factor mod n
 * 
 * After enough iterations, this should converge to a fixed point
 * that depends only on x mod n.
 */
int64_t FGG_Cassini_Mod(int64_t x, int64_t n, int depth = 5) {
    if (n <= 0) return 0;
    
    int64_t current = ((x % n) + n) % n;
    if (current == 0) return 0;
    
    for (int d = 1; d <= depth; d++) {
        auto [fk, fk1] = fib_pair(d, n);
        int64_t fk_prev = (fk1 - fk + n) % n;  // F_{k-1} = F_{k+1} - F_k
        int64_t factor = (fk_prev * fk1) % n;   // F_{k-1} * F_{k+1}
        
        // Cassini: factor = F_k^2 + (-1)^k
        current = (current * factor) % n;
    }
    
    return current;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  FGG_Mod via CASSINI COLLAPSE (Integer Version)\n";
    std::cout << "  F_{k-1} * F_{k+1} - F_k^2 = (-1)^k → factor = F_k^2 + (-1)^k\n";
    std::cout << "================================================================================\n\n";
    
    struct Test {
        int64_t x, n;
    };
    
    Test tests[] = {
        {17, 5}, {42, 5}, {100, 7}, {256, 16},
        {-17, 5}, {0, 5}, {12345, 97},
        {42, 13}, {100, 11}, {-100, 7},
    };
    
    std::cout << std::left << std::setw(8) << "x"
              << std::setw(6) << "n"
              << std::setw(12) << "Expected"
              << std::setw(12) << "FGG_Mod"
              << "Match\n";
    std::cout << std::string(45, '-') << "\n";
    
    int passed = 0;
    for (auto& t : tests) {
        int64_t expected = ((t.x % t.n) + t.n) % t.n;
        int64_t result = FGG_Cassini_Mod(t.x, t.n, 5);
        bool ok = (result == expected);
        if (ok) passed++;
        
        std::cout << std::left << std::setw(8) << t.x
                  << std::setw(6) << t.n
                  << std::setw(12) << expected
                  << std::setw(12) << result
                  << (ok ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n  Passed: " << passed << "/" << (sizeof(tests)/sizeof(tests[0])) << "\n\n";
    
    // Show what's happening for a specific case
    std::cout << "  Trace for 17 mod 5:\n";
    int64_t cur = 17 % 5;
    for (int d = 1; d <= 5; d++) {
        auto [fk, fk1] = fib_pair(d, 5);
        int64_t fk_prev = (fk1 - fk + 5) % 5;
        int64_t factor = (fk_prev * fk1) % 5;
        int64_t prev = cur;
        cur = (cur * factor) % 5;
        std::cout << "    d=" << d << ": F_" << (d-1) << "=" << fk_prev 
                  << " F_" << (d+1) << "=" << fk1 
                  << " factor=" << factor 
                  << " " << prev << " -> " << cur << "\n";
    }
    std::cout << "  Expected: 2, Got: " << cur << "\n";
    
    return 0;
}
