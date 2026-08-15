// FULL ADDER — COMPLETE (Sum AND Carry Out)
#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "FULL ADDER — COMPLETE TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    io.obfuscate_circuit_begin(3);  // a(0), b(1), cin(2)
    
    // XOR(a,b)
    int g1 = io.circuit_add_nand(0, 1);
    int g2 = io.circuit_add_nand(0, g1);
    int g3 = io.circuit_add_nand(1, g1);
    int xor_ab = io.circuit_add_nand(g2, g3);
    
    // SUM = XOR(xor_ab, cin)
    int g4 = io.circuit_add_nand(xor_ab, 2);
    int g5 = io.circuit_add_nand(xor_ab, g4);
    int g6 = io.circuit_add_nand(2, g4);
    int sum = io.circuit_add_nand(g5, g6);
    
    // COUT = (a AND b) OR (cin AND xor_ab)
    int g7 = io.circuit_add_nand(0, 1);
    int and_ab = io.circuit_add_nand(g7, g7);
    int g8 = io.circuit_add_nand(2, xor_ab);
    int and_cx = io.circuit_add_nand(g8, g8);
    int g9 = io.circuit_add_nand(and_ab, and_ab);
    int g10 = io.circuit_add_nand(and_cx, and_cx);
    int cout = io.circuit_add_nand(g9, g10);
    
    // Register BOTH outputs
    io.add_output(sum);
    io.add_output(cout);
    
    std::cout << "Circuit: " << (io.circuit_size()) << " NAND gates\n";
    std::cout << "Outputs: sum, cout\n\n";
    
    std::vector<std::vector<bool>> inputs;
    for (int i = 0; i < 8; i++) {
        inputs.push_back({(bool)(i & 4), (bool)(i & 2), (bool)(i & 1)});
    }
    
    int pass = 0;
    std::cout << "A B Cin | Sum | Cout | Status\n";
    std::cout << "--------|-----|------|-------\n";
    
    for (auto& in : inputs) {
        auto outputs = io.evaluate_multi(in);
        bool expected_sum = in[0] ^ in[1] ^ in[2];
        bool expected_cout = (in[0] && in[1]) || (in[2] && (in[0] ^ in[1]));
        
        bool ok = (outputs[0] == expected_sum && outputs[1] == expected_cout);
        pass += ok;
        
        std::cout << in[0] << " " << in[1] << " " << in[2] << "   |  "
                  << outputs[0] << "  |  " << outputs[1] << "   |  "
                  << (ok ? "✓" : "✗") << "\n";
    }
    
    std::cout << "\nResult: " << pass << "/8 PASS\n";
    return pass == 8 ? 0 : 1;
}
