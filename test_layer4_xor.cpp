#include <iostream>
#include <iomanip>
#include <vector>

// Simple XOR via direct NAND
int nand_gate(int a, int b) { return !(a && b); }

int xor_direct(int x, int y) {
    int n1 = nand_gate(x, y);
    int n2 = nand_gate(x, n1);
    int n3 = nand_gate(y, n1);
    return nand_gate(n2, n3);
}

int main() {
    std::cout << "Direct XOR via 4 NAND gates:\n";
    std::cout << "x y | XOR\n";
    std::cout << "---------\n";
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            std::cout << x << " " << y << " | " << xor_direct(x, y) << "\n";
        }
    }
    
    // C2 was computing XNOR — just NOT the output to get XOR
    std::cout << "\nXNOR + NOT = XOR? Check:";
    std::cout << (xor_direct(0,0) == 1 ? "FAIL" : "OK") << " ";
    std::cout << (xor_direct(0,1) == 1 ? "OK" : "FAIL") << " ";
    std::cout << (xor_direct(1,0) == 1 ? "OK" : "FAIL") << " ";
    std::cout << (xor_direct(1,1) == 1 ? "FAIL" : "OK") << "\n";
    
    return 0;
}
