// GENERAL CIRCUIT iO TEST V2 — Corrected wire indices
#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "GENERAL CIRCUIT iO TEST V2\n";
    std::cout << "=========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // ============ TEST 1: FULL ADDER ============
    std::cout << "1. FULL ADDER (corrected)\n";
    
    io.obfuscate_circuit_begin(3);  // 3 inputs: a(0), b(1), cin(2)
    
    // Gates build sequentially (wire indices auto-assigned)
    int w3 = io.circuit_add_nand(0, 1);      // nand_ab
    int w4 = io.circuit_add_nand(0, w3);     // nand_a_ab
    int w5 = io.circuit_add_nand(1, w3);     // nand_b_ab
    int w6 = io.circuit_add_nand(w4, w5);    // xor_ab
    int w7 = io.circuit_add_nand(w6, 2);     // nand_xc
    int w8 = io.circuit_add_nand(w6, w7);    // nand_x_xc
    int w9 = io.circuit_add_nand(2, w7);     // nand_c_xc
    int w10 = io.circuit_add_nand(w8, w9);   // sum
    
    // Register sum as output
    io.add_output(w10);
    
    std::cout << "   Wires: " << w3 << "," << w4 << "," << w5 << "," << w6 << ","
              << w7 << "," << w8 << "," << w9 << "," << w10 << "\n\n";
    
    std::vector<std::vector<bool>> inputs;
    for (int i = 0; i < 8; i++) {
        inputs.push_back({(bool)(i & 4), (bool)(i & 2), (bool)(i & 1)});
    }
    
    bool adder_pass = true;
    for (auto& in : inputs) {
        bool expected = in[0] ^ in[1] ^ in[2];
        bool result = io.evaluate(in);
        std::cout << "   " << in[0] << in[1] << in[2] << " → sum=" << result 
                  << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
        adder_pass &= (result == expected);
    }
    std::cout << "   Full Adder: " << (adder_pass ? "ALL 8 PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ TEST 2: 2-BIT COMPARATOR ============
    std::cout << "2. 2-BIT COMPARATOR\n";
    
    io.obfuscate_circuit_begin(4);  // 4 inputs: a0(0), b0(1), a1(2), b1(3)
    
    int c3 = io.circuit_add_nand(0, 1);       // nand(a0,b0)
    int c4 = io.circuit_add_nand(0, c3);      // nand(a0, nand)
    int c5 = io.circuit_add_nand(1, c3);      // nand(b0, nand)
    int c6 = io.circuit_add_nand(c4, c5);     // xor0
    int c7 = io.circuit_add_nand(c6, c6);     // xnor0 = NOT(xor0)
    
    int c8 = io.circuit_add_nand(2, 3);       // nand(a1,b1)
    int c9 = io.circuit_add_nand(2, c8);      // nand(a1, nand)
    int c10 = io.circuit_add_nand(3, c8);     // nand(b1, nand)
    int c11 = io.circuit_add_nand(c9, c10);   // xor1
    int c12 = io.circuit_add_nand(c11, c11);  // xnor1 = NOT(xor1)
    
    int c13 = io.circuit_add_nand(c7, c12);   // nand(xnor0, xnor1)
    int c14 = io.circuit_add_nand(c13, c13);  // equal = AND(xnor0, xnor1)
    
    io.add_output(c14);
    
    std::cout << "   Testing 16 combinations...\n";
    bool comp_pass = true;
    for (int i = 0; i < 16; i++) {
        bool a0 = (i & 8), b0 = (i & 4), a1 = (i & 2), b1 = (i & 1);
        std::vector<bool> input = {a0, b0, a1, b1};
        bool result = io.evaluate(input);
        bool expected = (a0 == b0) && (a1 == b1);
        
        if (result != expected) {
            std::cout << "   MISMATCH: " << a0 << b0 << " vs " << a1 << b1 
                      << " → got " << result << ", exp " << expected << "\n";
        }
        comp_pass &= (result == expected);
    }
    std::cout << "   Comparator: " << (comp_pass ? "ALL 16 PASS ✓" : "FAIL ✗") << "\n";
    
    return (adder_pass && comp_pass) ? 0 : 1;
}
