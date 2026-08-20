#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINARY FIELD Z_2 ANALYSIS\n";
    std::cout << "  NAND sa GF(2) na may fresh noise\n";
    std::cout << "========================================\n\n";

    // Sa GF(2), ang NAND ay:
    // NAND(x,y) = 1 + xy (mod 2)
    // 0 NAND 0 = 1
    // 0 NAND 1 = 1
    // 1 NAND 0 = 1
    // 1 NAND 1 = 0
    
    // Ang period-2 property:
    // NAND(NAND(x,x), NAND(x,x)) = x
    // Sa GF(2): 1 + (1+x)(1+x) = 1 + 1 + 2x + x² = x² + 2x
    // = x² (dahil 2x = 0 sa GF(2))
    // = x (dahil x² = x sa GF(2))
    
    std::cout << "1. NAND SA GF(2):\n";
    std::cout << "   NAND(x,y) = 1 + xy mod 2\n";
    std::cout << "-----------------------------------\n";
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            int nand = 1 + x * y;
            nand %= 2;
            std::cout << "   NAND(" << x << "," << y << ") = " << nand << "\n";
        }
    }
    
    std::cout << "\n2. PERIOD-2 SA GF(2):\n";
    std::cout << "   NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "-----------------------------------\n";
    
    for (int x = 0; x <= 1; x++) {
        int inner = (1 + x * x) % 2;
        int outer = (1 + inner * inner) % 2;
        std::cout << "   x=" << x << " -> " << outer 
                  << " (expected " << x << ")\n";
    }
    
    std::cout << "\n3. NOISE STRUCTURE SA GF(2):\n";
    std::cout << "   Sa GF(2), ang noise ay BIT\n";
    std::cout << "   Walang magnitude, may direction lang\n";
    std::cout << "   Noise + Noise = 0 (cancellation!)\n";
    std::cout << "-----------------------------------\n";
    
    // Sa GF(2), ang noise ay XOR
    // e1 XOR e2 XOR e1 = e2 (may cancellation)
    // Hindi tulad ng integer noise na additive
    
    std::cout << "   Integer noise: e1 + e2 + e1 = 2e1 + e2\n";
    std::cout << "   GF(2) noise:   e1 XOR e2 XOR e1 = e2\n";
    std::cout << "   (May cancellation sa GF(2)!)\n\n";
    
    std::cout << "4. ANG SUSI:\n";
    std::cout << "   Kung gagamit tayo ng binary FHE\n";
    std::cout << "   (tulad ng TFHE o FHEW),\n";
    std::cout << "   ang noise ay may XOR structure\n";
    std::cout << "   na may natural cancellation\n";
    std::cout << "-----------------------------------\n";
    
    // Sa TFHE, ang bootstrapping ay native
    // at ginagawa sa bawat gate evaluation
    std::cout << "   TFHE: bootstrapping sa bawat NAND\n";
    std::cout << "   FHEW: mas mabilis na variant\n";
    std::cout << "   BOTH: arbitrary depth na native\n\n";
    
    std::cout << "========================================\n";
    std::cout << "  KONKLUSYON:\n";
    std::cout << "  - Ang GF(2) ay may XOR noise na\n";
    std::cout << "    may natural cancellation\n";
    std::cout << "  - Ang TFHE/FHEW ay gumagamit nito\n";
    std::cout << "    para sa arbitrary depth\n";
    std::cout << "  - Hindi natin ito ma-replicate sa BFV\n";
    std::cout << "    dahil integer noise ang gamit\n";
    std::cout << "========================================\n";

    return 0;
}
