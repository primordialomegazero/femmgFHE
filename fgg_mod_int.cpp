#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

/**
 * FGG on INTEGERS modulo n.
 * Not real numbers — this operates on residue classes.
 * 
 * φ·ψ = -1. In Z_n: φ·ψ ≡ -1 (mod n).
 * The collapse property depends on n.
 */
int64_t FGG_Int(int64_t v, int64_t n, int depth = 3) {
    // Compute φ and ψ as integers modulo n
    // φ = (1 + sqrt(5))/2, but in Z_n we need the actual Fibonacci structure
    // F(k+1) * F(k-1) - F(k)^2 = (-1)^k  (Cassini identity)
    // This IS the integer version of φ·ψ = -1
    
    int64_t current = v % n;
    if (current < 0) current += n;
    
    for (int d = 0; d < depth; d++) {
        // Use Fibonacci recurrence: F_{k+1} * F_{k-1} - F_k^2 = (-1)^k
        // This preserves the Cassini invariant in integers
        int64_t f_k = 1, f_k1 = 1;
        for (int i = 2; i <= d + 2; i++) {
            int64_t tmp = (f_k + f_k1) % n;
            f_k = f_k1;
            f_k1 = tmp;
        }
        
        // Collapse: multiply by the ratio and take absolute value mod n
        // current * F_{k+1} * F_{k-1} ≡ current * (F_k^2 + (-1)^k) (mod n)
        int64_t f_prev = (f_k1 - f_k + n) % n;  // F_{k-1}
        int64_t factor = (f_k1 * f_prev) % n;   // F_{k+1} * F_{k-1}
        current = (current * factor) % n;
        
        // Cassini: F_{k+1} * F_{k-1} - F_k^2 = (-1)^k
        // So factor ≡ F_k^2 + (-1)^k (mod n)
        // After depth 3, this converges to a canonical form
    }
    
    return current;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  FGG ON INTEGERS MODULO N\n";
    std::cout << "  Using Cassini identity: F_{k+1} * F_{k-1} - F_k^2 = (-1)^k\n";
    std::cout << "================================================================================\n\n";
    
    struct Test {
        int64_t x, n;
        int64_t expected;  // x mod n
    };
    
    Test tests[] = {
        {17, 5, 2},
        {42, 5, 2},
        {100, 7, 2},
        {-17, 5, 3},  // In math: -17 mod 5 = 3 (canonical residue)
        {0, 5, 0},
        {12345, 97, 12345 % 97},
    };
    
    std::cout << "  Using Fibonacci-based collapse (not abs):\n\n";
    std::cout << std::left << std::setw(8) << "x"
              << std::setw(6) << "n"
              << std::setw(12) << "Expected"
              << std::setw(12) << "FGG_Int"
              << "Status\n";
    std::cout << std::string(45, '-') << "\n";
    
    for (auto& t : tests) {
        int64_t result = FGG_Int(t.x, t.n, 3);
        int64_t expected = ((t.x % t.n) + t.n) % t.n;
        bool ok = (result == expected);
        
        std::cout << std::left << std::setw(8) << t.x
                  << std::setw(6) << t.n
                  << std::setw(12) << expected
                  << std::setw(12) << result
                  << (ok ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n================================================================================\n";
    std::cout << "  This uses Fibonacci Cassini identity, not real-number φ.\n";
    std::cout << "  The question: does this converge to the canonical residue?\n";
    std::cout << "================================================================================\n";
    
    return 0;
}
