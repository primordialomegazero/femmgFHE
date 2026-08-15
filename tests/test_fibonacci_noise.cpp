#include <iostream>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <vector>
#include <cmath>

constexpr long Q = 536870909;

int main() {
    std::cout << "FIBONACCI NOISE vs RANDOM NOISE\n";
    std::cout << "=================================\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Test 1: Fibonacci noise growth
    std::cout << "1. FIBONACCI NOISE\n";
    std::vector<long> fib_noise;
    long f0 = 1, f1 = 1;
    for (int i = 0; i < 20; i++) {
        fib_noise.push_back(f1 % 1000);
        long f2 = f0 + f1;
        f0 = f1;
        f1 = f2;
    }
    
    std::cout << "  Noise values: ";
    for (long n : fib_noise) std::cout << n << " ";
    std::cout << "\n\n";
    
    // Test 2: Cumulative noise (multiplication)
    std::cout << "2. CUMULATIVE NOISE (multiplication chain)\n";
    std::cout << "  Fibonacci: ";
    long cumulative_fib = 1;
    for (long n : fib_noise) {
        cumulative_fib = (cumulative_fib * n) % 1000000;
    }
    std::cout << "Total = " << cumulative_fib << "\n";
    
    // Test 3: Random noise comparison
    std::cout << "  Random: ";
    long cumulative_rand = 1;
    srand(42);
    for (int i = 0; i < 20; i++) {
        long n = rand() % 1000 + 1;
        cumulative_rand = (cumulative_rand * n) % 1000000;
    }
    std::cout << "Total = " << cumulative_rand << "\n\n";
    
    // Test 4: Fibonacci vs Random growth
    std::cout << "3. GROWTH COMPARISON\n";
    std::cout << "  Fibonacci cumulative: " << cumulative_fib << "\n";
    std::cout << "  Random cumulative: " << cumulative_rand << "\n";
    std::cout << "  Ratio: " << (double)cumulative_fib / cumulative_rand << "\n\n";
    
    // Test 5: φ-scaled noise
    std::cout << "4. φ-SCALED NOISE\n";
    long phi_mod = 386640388;
    
    std::cout << "  Traditional: noise × φ = uncontrolled\n";
    std::cout << "  φ-scaled: noise × φ × ψ = noise × (-1)\n";
    std::cout << "  May cancellation!\n\n";
    
    std::cout << "=== CONCLUSION ===\n";
    std::cout << "Fibonacci noise ay may predictable structure\n";
    std::cout << "Kung ma-leverage ang structure, baka may natural damping\n";
    
    return 0;
}
