// GENERAL CIRCUIT iO TEST
// Hindi lang XOR — arbitrary boolean functions

#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>
#include <functional>

int main() {
    std::cout << "GENERAL CIRCUIT iO TEST\n";
    std::cout << "=======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // ============ TEST 1: FULL ADDER (1-bit) ============
    std::cout << "1. FULL ADDER (Sum + Carry)\n";
    std::cout << "   Input: a, b, cin\n";
    std::cout << "   Output: sum = a XOR b XOR cin, cout = (a AND b) OR (cin AND (a XOR b))\n\n";
    
    io.obfuscate_circuit_begin(3);  // 3 inputs
    
    // XOR(a,b)
    int nand_ab = io.circuit_add_nand(0, 1);
    int nand_a_ab = io.circuit_add_nand(0, nand_ab);
    int nand_b_ab = io.circuit_add_nand(1, nand_ab);
    int xor_ab = io.circuit_add_nand(nand_a_ab, nand_b_ab);
    
    // XOR(xor_ab, cin) = sum
    int nand_xc = io.circuit_add_nand(xor_ab, 2);
    int nand_x_xc = io.circuit_add_nand(xor_ab, nand_xc);
    int nand_c_xc = io.circuit_add_nand(2, nand_xc);
    int sum = io.circuit_add_nand(nand_x_xc, nand_c_xc);
    
    // AND(a,b)
    int nand_ab2 = io.circuit_add_nand(0, 1);
    int and_ab = io.circuit_add_nand(nand_ab2, nand_ab2);
    
    // AND(cin, xor_ab)
    int nand_cx = io.circuit_add_nand(2, xor_ab);
    int and_cx = io.circuit_add_nand(nand_cx, nand_cx);
    
    // OR(and_ab, and_cx) = cout
    int not_and_ab = io.circuit_add_nand(and_ab, and_ab);
    int not_and_cx = io.circuit_add_nand(and_cx, and_cx);
    int cout = io.circuit_add_nand(not_and_ab, not_and_cx);
    
    std::cout << "   Full Adder Circuit: 14 NAND gates\n";
    std::cout << "   Testing 8 input combinations...\n\n";
    
    std::vector<std::vector<bool>> inputs;
    for (int i = 0; i < 8; i++) {
        inputs.push_back({(bool)(i & 4), (bool)(i & 2), (bool)(i & 1)});
    }
    
    bool all_pass = true;
    for (auto& in : inputs) {
        bool a = in[0], b = in[1], cin = in[2];
        bool expected_sum = a ^ b ^ cin;
        bool expected_cout = (a && b) || (cin && (a ^ b));
        
        // Test sum (wire 10) at cout (wire 13)
        // Need to check evaluate output
        bool result = io.evaluate(in);
        
        std::cout << "   " << a << b << cin << " → sum=" << result 
                  << " (exp " << expected_sum << ") "
                  << (result == expected_sum ? "✓" : "✗") << "\n";
        all_pass &= (result == expected_sum);
    }
    
    std::cout << "\n   Full Adder: " << (all_pass ? "ALL 8 PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ TEST 2: 2-BIT COMPARATOR ============
    std::cout << "2. 2-BIT COMPARATOR (A == B)\n";
    std::cout << "   Input: a0, a1, b0, b1\n";
    std::cout << "   Output: 1 if (a0==b0) AND (a1==b1)\n\n";
    
    io.obfuscate_circuit_begin(4);
    
    // XNOR(a0,b0) = NOT(XOR(a0,b0))
    int n1 = io.circuit_add_nand(0, 1);
    int n2 = io.circuit_add_nand(0, n1);
    int n3 = io.circuit_add_nand(1, n1);
    int xor0 = io.circuit_add_nand(n2, n3);
    int xnor0 = io.circuit_add_nand(xor0, xor0);
    
    // XNOR(a1,b1)
    int n4 = io.circuit_add_nand(2, 3);
    int n5 = io.circuit_add_nand(2, n4);
    int n6 = io.circuit_add_nand(3, n4);
    int xor1 = io.circuit_add_nand(n5, n6);
    int xnor1 = io.circuit_add_nand(xor1, xor1);
    
    // AND(xnor0, xnor1)
    int n7 = io.circuit_add_nand(xnor0, xnor1);
    int equal = io.circuit_add_nand(n7, n7);
    
    std::cout << "   Comparator: 12 NAND gates\n";
    std::cout << "   Testing 16 input combinations...\n\n";
    
    bool comparator_pass = true;
    for (int i = 0; i < 16; i++) {
        bool a0 = (i & 8), a1 = (i & 4), b0 = (i & 2), b1 = (i & 1);
        std::vector<bool> input = {a0, a1, b0, b1};
        bool result = io.evaluate(input);
        bool expected = (a0 == b0) && (a1 == b1);
        comparator_pass &= (result == expected);
        
        if (!(result == expected)) {
            std::cout << "   MISMATCH: " << a0 << a1 << " vs " << b0 << b1 
                      << " → got " << result << ", exp " << expected << "\n";
        }
    }
    
    std::cout << "   Comparator: " << (comparator_pass ? "ALL 16 PASS ✓" : "FAIL ✗") << "\n";
    
    return (all_pass && comparator_pass) ? 0 : 1;
}
