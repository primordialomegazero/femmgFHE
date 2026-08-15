#include <iostream>
#include <NTL/ZZ_p.h>

constexpr long Q = 536870909;
constexpr long PHI_MOD_Q = 386640388;

long fib_mod(long n, long mod) {
    if (n == 0) return 0;
    long a = 0, b = 1;
    for (long i = 2; i <= n; i++) {
        long c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

long phi_pow_mod(long n, long mod) {
    long result = 1;
    for (long i = 0; i < n; i++) {
        result = (result * PHI_MOD_Q) % mod;
    }
    return result;
}

int main() {
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    long n = 42;
    long s = phi_pow_mod(n, Q);
    long s_sq = (s * s) % Q;
    long s_plus_1 = (s + 1) % Q;
    
    std::cout << "n = " << n << "\n";
    std::cout << "s = φ^n = " << s << "\n";
    std::cout << "s² = φ^(2n) = " << s_sq << "\n";
    std::cout << "s+1 = " << s_plus_1 << "\n";
    std::cout << "s² == s+1? " << (s_sq == s_plus_1 ? "YES" : "NO") << "\n\n";
    
    // Ang tamang relation para sa s = φ^n:
    // s² = φ^(2n) = φ^n · φ^n = s · s (tautology)
    // Kailangan natin ng α at β na s² = α·s + β
    
    // From φ^(2n) = F(2n)·φ + F(2n-1)
    // s = φ^n = F(n)·φ + F(n-1)
    // s² = φ^(2n) = F(2n)·φ + F(2n-1)
    
    long F_n = fib_mod(n, Q);
    long F_n_minus_1 = fib_mod(n-1, Q);
    long F_2n = fib_mod(2*n, Q);
    long F_2n_minus_1 = fib_mod(2*n-1, Q);
    
    std::cout << "F(n) = " << F_n << "\n";
    std::cout << "F(n-1) = " << F_n_minus_1 << "\n";
    std::cout << "F(2n) = " << F_2n << "\n";
    std::cout << "F(2n-1) = " << F_2n_minus_1 << "\n\n";
    
    // Verify: s = F(n)·φ + F(n-1)
    long s_from_fib = (F_n * PHI_MOD_Q + F_n_minus_1) % Q;
    std::cout << "s from Fibonacci: " << s_from_fib << "\n";
    std::cout << "s from φ^n: " << s << "\n";
    std::cout << "Match: " << (s == s_from_fib ? "YES" : "NO") << "\n\n";
    
    // Verify: s² = F(2n)·φ + F(2n-1)
    long s_sq_from_fib = (F_2n * PHI_MOD_Q + F_2n_minus_1) % Q;
    std::cout << "s² from Fibonacci: " << s_sq_from_fib << "\n";
    std::cout << "s² direct: " << s_sq << "\n";
    std::cout << "Match: " << (s_sq == s_sq_from_fib ? "YES" : "NO") << "\n\n";
    
    // Ngayon, express s² in terms of s:
    // s² = α·s + β
    // F(2n)·φ + F(2n-1) = α·(F(n)·φ + F(n-1)) + β
    // = α·F(n)·φ + α·F(n-1) + β
    // Kaya: α·F(n) = F(2n) at α·F(n-1) + β = F(2n-1)
    
    // α = F(2n) / F(n) mod Q
    long F_n_inv = 1;
    // Kailangan ng modular inverse ng F(n)
    for (long i = 1; i < Q; i++) {
        if ((F_n * i) % Q == 1) {
            F_n_inv = i;
            break;
        }
    }
    
    long alpha = (F_2n * F_n_inv) % Q;
    long beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n";
    std::cout << "Verify: s² == α·s + β? " 
              << (s_sq == (alpha * s + beta) % Q ? "YES" : "NO") << "\n";
    
    return 0;
}
