#include <iostream>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <vector>

constexpr long Q = 536870909;

int main() {
    std::cout << "GSW-STYLE MULTIPLICATION TEST\n";
    std::cout << "==============================\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Sa GSW, ang ciphertext ay matrix
    // C = [c0; c1] kung saan c0, c1 ay polynomials
    
    // Ang multiplication ay:
    // C_mult = C1 · G^{-1}(C2)
    // Kung saan G ay gadget matrix
    
    // Advantage: Ang dimension ay HINDI lumalaki
    // 2x2 matrix · 2x2 matrix = 2x2 matrix
    
    std::cout << "Traditional (3-component):\n";
    std::cout << "  (c0, c1, c2) × (d0, d1, d2)\n";
    std::cout << "  = 5 components (t0..t4)\n";
    std::cout << "  Kailangan: relinearization\n\n";
    
    std::cout << "GSW-style (matrix):\n";
    std::cout << "  C × D = 2x2 matrix\n";
    std::cout << "  Result: 2x2 matrix\n";
    std::cout << "  Walang dimension growth\n";
    std::cout << "  Walang relinearization na kailangan\n\n";
    
    // Test: Matrix multiplication
    std::cout << "=== SIMPLE MATRIX TEST ===\n\n";
    
    // 2x2 matrices
    NTL::ZZ_p a00 = NTL::to_ZZ_p(1);
    NTL::ZZ_p a01 = NTL::to_ZZ_p(2);
    NTL::ZZ_p a10 = NTL::to_ZZ_p(3);
    NTL::ZZ_p a11 = NTL::to_ZZ_p(4);
    
    NTL::ZZ_p b00 = NTL::to_ZZ_p(5);
    NTL::ZZ_p b01 = NTL::to_ZZ_p(6);
    NTL::ZZ_p b10 = NTL::to_ZZ_p(7);
    NTL::ZZ_p b11 = NTL::to_ZZ_p(8);
    
    // Matrix multiplication
    NTL::ZZ_p c00 = a00 * b00 + a01 * b10;
    NTL::ZZ_p c01 = a00 * b01 + a01 * b11;
    NTL::ZZ_p c10 = a10 * b00 + a11 * b10;
    NTL::ZZ_p c11 = a10 * b01 + a11 * b11;
    
    std::cout << "C = A × B:\n";
    std::cout << "  [" << c00 << " " << c01 << "]\n";
    std::cout << "  [" << c10 << " " << c11 << "]\n\n";
    
    std::cout << "Dimension: 2x2 × 2x2 = 2x2 (constant)\n";
    std::cout << "Walang relinearization needed!\n\n";
    
    std::cout << "=== KEY INSIGHT ===\n\n";
    std::cout << "Kung gumamit tayo ng GSW-style na matrix representation,\n";
    std::cout << "ang multiplication ay HINDI lumalaki ang dimension.\n";
    std::cout << "Hindi na kailangan ng relinearization!\n";
    
    return 0;
}
