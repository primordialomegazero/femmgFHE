// CORRECT NAND FORMULA
// NAND(a,b) = 1 - a·b (with φ/ψ encoding)

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ p = NTL::to_ZZ(3604481);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    
    std::cout << "CORRECT NAND TEST\n";
    std::cout << "=================\n\n";
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";
    
    // Test period-2
    std::cout << "PERIOD-2 CHAIN (20 depths):\n";
    NTL::ZZ current = phi;
    int errors = 0;
    
    for (int i = 0; i <= 20; i++) {
        NTL::ZZ expected = (i % 2 == 0) ? phi : psi;
        std::cout << "  Depth " << i << ": " << current << " (exp " << expected << ")";
        
        if (current != expected) {
            std::cout << " ✗";
            errors++;
        } else {
            std::cout << " ✓";
        }
        std::cout << "\n";
        
        // NAND(current, current) = 1 - current²
        NTL::ZZ nand = (NTL::to_ZZ(1) - current * current) % p;
        if (nand < 0) nand += p;
        current = nand;
    }
    
    std::cout << "\nErrors: " << errors << "/21\n";
    return 0;
}
