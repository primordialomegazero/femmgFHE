#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <cmath>

constexpr long Q = 536870909;
constexpr int N = 1024;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

// FGG: Fibonacci Golden Gate reduction
// X^N = X + 1 (sa halip na X^N = -1)
// Ito ay φ-analog sa polynomial ring
inline void fgg_reduce(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    
    NTL::ZZ_pX reduced;
    reduced.SetLength(N);
    
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(reduced, i, NTL::coeff(poly, i));
    }
    
    // X^N = X + 1
    // X^(N+k) = X^(k+1) + X^k
    for (int i = N; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        int k = i - N;
        
        // X^(N+k) = X^k · X^N = X^k · (X + 1) = X^(k+1) + X^k
        if (k + 1 < N) {
            NTL::SetCoeff(reduced, k + 1, NTL::coeff(reduced, k + 1) + coeff);
        }
        if (k < N) {
            NTL::SetCoeff(reduced, k, NTL::coeff(reduced, k) + coeff);
        }
    }
    
    poly = reduced;
}

// Test: Simple polynomial reduction
void test_simple_reduction() {
    std::cout << "=== SIMPLE FGG REDUCTION TEST ===\n\n";
    
    // Poly: X^1024 + 5·X + 3
    NTL::ZZ_pX poly;
    NTL::SetCoeff(poly, 1024, 1);
    NTL::SetCoeff(poly, 1, 5);
    NTL::SetCoeff(poly, 0, 3);
    
    std::cout << "Before: deg = " << NTL::deg(poly) << "\n";
    fgg_reduce(poly);
    std::cout << "After: deg = " << NTL::deg(poly) << "\n";
    
    // Expected: X^1024 = X + 1
    // Kaya: (X + 1) + 5X + 3 = 6X + 4
    std::cout << "coeff[0] = " << NTL::coeff(poly, 0) << " (expected 4)\n";
    std::cout << "coeff[1] = " << NTL::coeff(poly, 1) << " (expected 6)\n\n";
}

// Test: Chain multiplication na walang degree growth
void test_chain_multiplication() {
    std::cout << "=== CHAIN MULTIPLICATION TEST ===\n\n";
    
    // Gumawa ng dalawang polynomials na may degree 100
    NTL::ZZ_pX a, b;
    for (int i = 0; i < 100; i++) {
        NTL::SetCoeff(a, i, i + 1);
        NTL::SetCoeff(b, i, 100 - i);
    }
    
    std::cout << "a deg = " << NTL::deg(a) << "\n";
    std::cout << "b deg = " << NTL::deg(b) << "\n";
    
    // Multiply at i-reduce
    NTL::ZZ_pX product = a * b;
    std::cout << "a·b deg (before reduction) = " << NTL::deg(product) << "\n";
    
    fgg_reduce(product);
    std::cout << "a·b deg (after FGG reduction) = " << NTL::deg(product) << "\n\n";
    
    // Chain: 10 multiplications
    NTL::ZZ_pX result = a;
    for (int i = 0; i < 10; i++) {
        result = result * b;
        fgg_reduce(result);
    }
    
    std::cout << "After 10 multiplications: deg = " << NTL::deg(result) << "\n";
    std::cout << "Status: " << (NTL::deg(result) < N ? "BOUNDED ✓" : "UNBOUNDED ✗") << "\n";
}

int main() {
    init_ring();
    test_simple_reduction();
    test_chain_multiplication();
    return 0;
}
