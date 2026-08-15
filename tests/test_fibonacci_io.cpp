#include "../src/io/golden_fibonacci_io.h"
#include <iostream>

int main() {
    std::cout << "FIBONACCI iO TEST\n";
    std::cout << "=================\n\n";
    
    // 257-bit Q
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    GoldenFibonacciIO::FibonacciIO io(Q, 42);
    
    // Print security info
    io.print_security_info();
    std::cout << "\n";
    
    // Test: AND gate via truth table
    auto and_func = [](const std::vector<bool>& in) {
        return in[0] && in[1];
    };
    
    io.obfuscate_truth_table(and_func, 2, 12345);
    
    std::cout << "TRUTH TABLE MODE (AND gate):\n";
    std::vector<std::vector<bool>> test_inputs = {
        {false, false},
        {false, true},
        {true, false},
        {true, true}
    };
    
    for (auto& input : test_inputs) {
        bool result = io.evaluate(input);
        bool expected = input[0] && input[1];
        std::cout << "  AND(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ") " 
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    // Test: Circuit mode
    std::cout << "\nCIRCUIT MODE (NAND chain):\n";
    io.obfuscate_circuit_begin(2);
    int g1 = io.circuit_add_nand(0, 1, 2000);
    int g2 = io.circuit_add_nand(g1, g1, 2001);
    
    for (auto& input : test_inputs) {
        bool result = io.evaluate(input);
        bool nand_result = !(input[0] && input[1]);
        bool expected = !(nand_result && nand_result);
        std::cout << "  NOT(NAND(" << input[0] << "," << input[1] << ")) = " 
                  << result << " (expected " << expected << ") " 
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    std::cout << "\n=== FIBONACCI iO TEST COMPLETE ===\n";
    
    return 0;
}
