// DEBUG: iO Functional Equivalence
#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "iO EQUIVALENCE DEBUG\n";
    std::cout << "====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // Test Circuit 1: XOR via 4 NAND gates
    std::cout << "CIRCUIT 1 (NAND-based XOR):\n";
    io.obfuscate_circuit_begin(2);
    int nand_ab = io.circuit_add_nand(0, 1);
    int nand_a_ab = io.circuit_add_nand(0, nand_ab);
    int nand_b_ab = io.circuit_add_nand(1, nand_ab);
    int xor_out = io.circuit_add_nand(nand_a_ab, nand_b_ab);
    
    std::cout << "  Wire indices: " << nand_ab << ", " << nand_a_ab << ", " 
              << nand_b_ab << ", " << xor_out << "\n";
    
    // Test all 4 inputs
    std::vector<std::vector<bool>> inputs = {
        {false,false}, {false,true}, {true,false}, {true,true}
    };
    
    std::cout << "  Circuit 1 results:\n";
    for (auto& in : inputs) {
        bool result = io.evaluate(in);
        bool expected = in[0] != in[1];
        std::cout << "    XOR(" << in[0] << "," << in[1] << ") = " 
                  << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    // Re-obfuscate for Circuit 2
    std::cout << "\nCIRCUIT 2 (Alternative XOR):\n";
    io.obfuscate_circuit_begin(2);
    int not_a = io.circuit_add_nand(0, 0);
    int not_b = io.circuit_add_nand(1, 1);
    int and_nota_b = io.circuit_add_nand(not_a, 1);
    int and_a_notb = io.circuit_add_nand(0, not_b);
    int or_out = io.circuit_add_nand(and_nota_b, and_a_notb);
    int xor2_out = io.circuit_add_nand(or_out, or_out);
    
    std::cout << "  Wire indices: " << not_a << ", " << not_b << ", " 
              << and_nota_b << ", " << and_a_notb << ", " << or_out << ", " << xor2_out << "\n";
    
    std::cout << "  Circuit 2 results:\n";
    for (auto& in : inputs) {
        bool result = io.evaluate(in);
        bool expected = in[0] != in[1];
        std::cout << "    XOR(" << in[0] << "," << in[1] << ") = " 
                  << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    return 0;
}
