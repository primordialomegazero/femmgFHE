// φ-ψ DIRECT SA PLAINTEXT MODULUS
// I-compute ang φ at ψ directly sa 65537

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "φ-ψ DIRECT SA 65537\n";
    std::cout << "===================\n\n";

    NTL::ZZ p = NTL::to_ZZ(65537);

    // Hanapin ang √5 mod 65537
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    
    // May dalawang roots: sqrt5 at p-sqrt5
    // Verify na ang sqrt5 ay talagang sqrt ng 5
    std::cout << "   sqrt5² mod p = " << (sqrt5*sqrt5)%p << " (dapat 5)\n";

    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;

    std::cout << "1. DIRECT φ at ψ sa 65537:\n";
    std::cout << "   √5 mod 65537 = " << sqrt5 << "\n";
    std::cout << "   φ = " << phi << "\n";
    std::cout << "   ψ = " << psi << "\n\n";

    // Verify: φ² = φ+1 at ψ² = ψ+1
    std::cout << "2. VERIFY φ² = φ+1:\n";
    std::cout << "   φ² = " << (phi*phi) % p << "\n";
    std::cout << "   φ+1 = " << (phi+1) % p << "\n";
    std::cout << "   Match: " << (((phi*phi)%p == (phi+1)%p) ? "YES ✓" : "NO ✗") << "\n\n";

    std::cout << "3. VERIFY φ+ψ = 1:\n";
    std::cout << "   φ+ψ = " << (phi+psi) % p << "\n\n";

    // NAND(φ,φ) = 1 - φ² = 1 - (φ+1) = -φ = ψ
    NTL::ZZ nand = (NTL::to_ZZ(1) - (phi*phi)%p) % p;
    if (nand < 0) nand += p;
    std::cout << "4. NAND(φ,φ):\n";
    std::cout << "   1-φ² = " << nand << "\n";
    std::cout << "   ψ = " << psi << "\n";
    std::cout << "   Match: " << (nand == psi ? "YES ✓" : "NO ✗") << "\n";

    return 0;
}
