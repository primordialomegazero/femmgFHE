#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "AUTO KEY SWITCHING RESEARCH\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("18446744073709551611");
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    if (sqrt5 > Q/2) sqrt5 = Q - sqrt5;
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // Fibonacci at Lucas
    int k = 42;
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i <= 2*k; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    
    NTL::ZZ F_k = fib[k];
    NTL::ZZ F_2k = fib[2*k];
    NTL::ZZ F_k_minus_1 = fib[k-1];
    NTL::ZZ F_2k_minus_1 = fib[2*k-1];
    NTL::ZZ L_k = (F_2k * NTL::InvMod(F_k, Q)) % Q;
    
    // Random secret: s = r·φ^k
    NTL::ZZ r = NTL::to_ZZ("123456789");
    NTL::ZZ s = NTL::to_ZZ(1);
    for (int i = 0; i < k; i++) s = (s * phi) % Q;
    s = (s * r) % Q;
    
    // Auto key switching: hanapin ang α' at β' para sa random secret
    NTL::ZZ alpha_prime = (r * L_k) % Q;
    NTL::ZZ beta_prime = (r * r * F_2k_minus_1 - alpha_prime * r * F_k_minus_1) % Q;
    if (beta_prime < 0) beta_prime += Q;
    
    // Verify: s² = α'·s + β'
    NTL::ZZ s_sq = (s * s) % Q;
    NTL::ZZ asb = (alpha_prime * s + beta_prime) % Q;
    
    std::cout << "φ = " << phi << "\n";
    std::cout << "F(42) = " << F_k << "\n";
    std::cout << "L(42) = " << L_k << "\n\n";
    
    std::cout << "Random r = " << r << "\n";
    std::cout << "s = r·φ^42 = " << s << "\n";
    std::cout << "s² = " << s_sq << "\n";
    std::cout << "α'·s + β' = " << asb << "\n";
    std::cout << "Match: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Test: auto key switching para sa ibang random r
    r = NTL::to_ZZ("987654321");
    s = NTL::to_ZZ(1);
    for (int i = 0; i < k; i++) s = (s * phi) % Q;
    s = (s * r) % Q;
    
    alpha_prime = (r * L_k) % Q;
    beta_prime = (r * r * F_2k_minus_1 - alpha_prime * r * F_k_minus_1) % Q;
    if (beta_prime < 0) beta_prime += Q;
    
    s_sq = (s * s) % Q;
    asb = (alpha_prime * s + beta_prime) % Q;
    
    std::cout << "Second random r = " << r << "\n";
    std::cout << "s² = " << s_sq << "\n";
    std::cout << "α'·s + β' = " << asb << "\n";
    std::cout << "Match: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}
