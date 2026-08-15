// Debug: Circuit Rebuild Test
#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "CIRCUIT REBUILD DEBUG\n";
    std::cout << "=====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // Test: Simple XOR circuit
    io.obfuscate_circuit_begin(2);
    int g1 = io.circuit_add_nand(0, 1);  // Wire 2: NAND(a,b)
    int g2 = io.circuit_add_nand(0, g1); // Wire 3: NAND(a, NAND(a,b))
    int g3 = io.circuit_add_nand(1, g1); // Wire 4: NAND(b, NAND(a,b))
    int g4 = io.circuit_add_nand(g2, g3); // Wire 5: XOR
    
    std::cout << "Circuit wires: " << g1 << ", " << g2 << ", " << g3 << ", " << g4 << "\n";
    
    // Test lahat ng inputs
    std::vector<std::vector<bool>> inputs = {
        {false, false},
        {false, true},
        {true, false},
        {true, true}
    };
    
    for (auto& in : inputs) {
        bool result = io.evaluate(in);
        bool expected = in[0] != in[1];
        std::cout << "  XOR(" << in[0] << "," << in[1] << ") = " 
                  << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    // Check kung nagre-rebuild ba
    std::cout << "\nRebuild check:\n";
    for (int i = 0; i < 3; i++) {
        io.obfuscate_circuit_begin(2);
        io.circuit_add_nand(0, 1);
        io.circuit_add_nand(0, 2);
        io.circuit_add_nand(1, 2);
        io.circuit_add_nand(3, 4);
        
        bool result = io.evaluate({false, true});
        std::cout << "  Eval " << i << ": XOR(0,1) = " << result << " (exp 1)\n";
    }
    
    return 0;
}
