#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

int main() {
    init_ring();
    
    std::cout << "SECRET KEY INVERSE VIA φ\n\n";
    
    // Sa RLWE, kailangan natin ng s² reduction
    // Pero ano kung may natural na inverse ang s?
    
    // Sa φ world: φ·ψ = -1 → ψ = -φ^(-1)
    // Kaya ang inverse ng φ ay -ψ
    
    // Ano kung ang s ay may φ-like structure?
    // s·s^(-1) = 1 (trivial)
    // Pero paano ito nakakatulong sa s² reduction?
    
    // IDEA: Kung ang decryption ay gumagamit ng s^(-1) sa halip na s
    // Ang multiplication ay magbibigay ng ibang structure
    
    std::cout << "Traditional decryption: [1, s] · [c0; c1]\n";
    std::cout << "  Kailangan ng s² sa multiplication\n\n";
    
    std::cout << "Alternative: [1, s^(-1)] · [c0; c1]\n";
    std::cout << "  Multiplication: (c0 + c1·s^(-1)) × (d0 + d1·s^(-1))\n";
    std::cout << "  = c0·d0 + (c0·d1 + c1·d0)·s^(-1) + c1·d1·s^(-2)\n";
    std::cout << "  s^(-2) = (s^(-1))² — same problem, iba lang ang variable\n\n";
    
    // TEST: May natural bang reduction sa s?
    std::cout << "=== TEST: NTT-friendly na may inverse ===\n";
    
    // Sa ring X^N + 1, ang X ay may inverse: X^(-1) = -X^(N-1)
    // Kasi X · (-X^(N-1)) = -X^N = -(-1) = 1
    
    NTL::ZZ_pX x;
    NTL::SetCoeff(x, 1, 1);  // X
    
    NTL::ZZ_pX x_inv;
    NTL::SetCoeff(x_inv, N-1, -1);  // -X^(N-1)
    
    NTL::ZZ_pX product = x * x_inv;
    // Reduce: X^N = -1 → X·(-X^(N-1)) = -X^N = 1
    
    std::cout << "X · X^(-1) = " << NTL::coeff(product, 0) << " (expected 1)\n\n";
    
    // KEY: Ang X ay may simple inverse sa ring
    // Ano kung ang s ay may similar na simple inverse?
    
    std::cout << "=== KEY QUESTION ===\n";
    std::cout << "Ano kung gumawa tayo ng s na ang inverse ay KNOWN?\n";
    std::cout << "Hindi kailangan i-encrypt ang s² kung ang s^(-1) ay public\n\n";
    
    std::cout << "Approach: s = X (o ibang simple polynomial)\n";
    std::cout << "  - s^(-1) ay computable\n";
    std::cout << "  - s² = X² (degree 2, maliit)\n";
    std::cout << "  - Walang malaking ciphertext growth\n";
    
    return 0;
}
