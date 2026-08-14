#include <iostream>
#include <vector>

int main() {
    std::cout << "4-input XOR Circuit Debug\n\n";
    
    // Current XOR4 circuit:
    // w4 = NAND(a,b)
    // w5 = NAND(c,d)
    // w6 = NAND(w4,w5)
    // w7 = NAND(w4,w6)
    // w8 = NAND(w5,w6)
    // w9 = NAND(w7,w8)
    
    // Test sa isang specific case
    bool a = false, b = false, c = false, d = false;
    
    bool w4 = !(a && b);         // NAND(a,b) = 1
    bool w5 = !(c && d);         // NAND(c,d) = 1
    bool w6 = !(w4 && w5);       // NAND(1,1) = 0
    bool w7 = !(w4 && w6);       // NAND(1,0) = 1
    bool w8 = !(w5 && w6);       // NAND(1,0) = 1
    bool w9 = !(w7 && w8);       // NAND(1,1) = 0
    
    std::cout << "Test (0,0,0,0):\n";
    std::cout << "  w4 = NAND(0,0) = " << w4 << "\n";
    std::cout << "  w5 = NAND(0,0) = " << w5 << "\n";
    std::cout << "  w6 = NAND(1,1) = " << w6 << "\n";
    std::cout << "  w7 = NAND(1,0) = " << w7 << "\n";
    std::cout << "  w8 = NAND(1,0) = " << w8 << "\n";
    std::cout << "  w9 = NAND(1,1) = " << w9 << "\n";
    std::cout << "  Expected XOR4 = 0\n\n";
    
    // Ang problema: ang circuit na ito ay hindi 4-input XOR
    // Ito ay 2-level NAND tree na hindi tama para sa XOR
    
    // Tamang 4-input XOR:
    // XOR(a,b,c,d) = XOR(XOR(a,b), XOR(c,d))
    // XOR(x,y) = NAND(NAND(x, NAND(x,y)), NAND(y, NAND(x,y)))
    
    // Mas simpleng approach: cascade ng 2-input XOR
    // XOR4 = XOR(XOR(a,b), XOR(c,d))
    
    // XOR(a,b) = NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    
    bool w4b = !(a && b);           // NAND(a,b)
    bool w5b = !(a && w4b);         // NAND(a, NAND(a,b))
    bool w6b = !(b && w4b);         // NAND(b, NAND(a,b))
    bool xor_ab = !(w5b && w6b);    // NAND(w5,w6) = XOR(a,b)
    
    bool w7b = !(c && d);           // NAND(c,d)
    bool w8b = !(c && w7b);         // NAND(c, NAND(c,d))
    bool w9b = !(d && w7b);         // NAND(d, NAND(c,d))
    bool xor_cd = !(w8b && w9b);    // NAND(w8,w9) = XOR(c,d)
    
    bool w10 = !(xor_ab && xor_cd);  // NAND(XORab, XORcd)
    bool w11 = !(xor_ab && w10);     // NAND(XORab, NAND)
    bool w12 = !(xor_cd && w10);     // NAND(XORcd, NAND)
    bool xor4 = !(w11 && w12);       // XOR(XORab, XORcd)
    
    std::cout << "Correct XOR4 circuit:\n";
    std::cout << "  XOR(a,b) = " << xor_ab << "\n";
    std::cout << "  XOR(c,d) = " << xor_cd << "\n";
    std::cout << "  XOR4 = " << xor4 << "\n";
    std::cout << "  Expected = " << (a ^ b ^ c ^ d) << "\n";
    
    return 0;
}
