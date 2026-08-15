#include <iostream>
#include <cmath>

constexpr long Q = 536870909;
constexpr long PHI_MOD_Q = 386640388;
constexpr long PSI_MOD_Q = 150230522;

// Fibonacci modulo Q
long fib_mod(long n, long mod) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    long a = 0, b = 1;
    for (long i = 2; i <= n; i++) {
        long c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

int main() {
    std::cout << "FIBONACCI SWITCHING TEST\n\n";
    
    // Test: φ^n = F(n)·φ + F(n-1)
    for (int n = 1; n <= 10; n++) {
        long fn = fib_mod(n, Q);
        long fn_minus_1 = fib_mod(n - 1, Q);
        
        // φ^n mod Q
        long phi_n = 1;
        long phi_val = PHI_MOD_Q;
        for (int i = 0; i < n; i++) {
            phi_n = (phi_n * phi_val) % Q;
        }
        
        // F(n)·φ + F(n-1)
        long fib_form = (fn * phi_val + fn_minus_1) % Q;
        
        std::cout << "n=" << n << ": φ^n=" << phi_n 
                  << " F(n)φ+F(n-1)=" << fib_form
                  << (phi_n == fib_form ? " ✓" : " ✗") << "\n";
    }
    
    std::cout << "\n";
    
    // Test: Automatic switching
    std::cout << "=== AUTOMATIC SWITCHING ===\n";
    std::cout << "Kung s = φ^n, ang s² = φ^(2n)\n";
    std::cout << "φ^(2n) = F(2n)·φ + F(2n-1)\n";
    std::cout << "Kaya ang reduction ay:\n";
    std::cout << "  c1·d1·φ^(2n) = c1·d1·(F(2n)·φ + F(2n-1))\n";
    std::cout << "  = F(2n)·(c1·d1·φ) + F(2n-1)·(c1·d1)\n\n";
    
    std::cout << "Ang φ^(2n) ay automatic na nagre-reduce\n";
    std::cout << "sa φ at constant terms!\n";
    
    return 0;
}
