// 4-BIT RIPPLE CARRY ADDER
// 4 Full Adders chained — 256 combinations

#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "4-BIT RIPPLE CARRY ADDER — COMPLETE\n";
    std::cout << "=====================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // 8 inputs: A0(0), A1(1), A2(2), A3(3), B0(4), B1(5), B2(6), B3(7)
    io.obfuscate_circuit_begin(8);
    
    // Full Adder 1 (least significant bit)
    int fa1_g1 = io.circuit_add_nand(0, 4);       // NAND(A0,B0)
    int fa1_g2 = io.circuit_add_nand(0, fa1_g1);  // NAND(A0, NAND)
    int fa1_g3 = io.circuit_add_nand(4, fa1_g1);  // NAND(B0, NAND)
    int fa1_xor = io.circuit_add_nand(fa1_g2, fa1_g3);  // XOR(A0,B0)
    int fa1_g4 = io.circuit_add_nand(fa1_xor, fa1_xor); // NOT(XOR)
    int s0 = io.circuit_add_nand(fa1_g4, fa1_g4);  // S0 = XOR(A0,B0) (no carry in)
    
    // Carry out ng FA1
    int fa1_g5 = io.circuit_add_nand(0, 4);
    int cout1 = io.circuit_add_nand(fa1_g5, fa1_g5);  // COUT1 = A0 AND B0
    
    // Full Adder 2
    int fa2_g1 = io.circuit_add_nand(1, 5);
    int fa2_g2 = io.circuit_add_nand(1, fa2_g1);
    int fa2_g3 = io.circuit_add_nand(5, fa2_g1);
    int fa2_xor = io.circuit_add_nand(fa2_g2, fa2_g3);  // XOR(A1,B1)
    int fa2_g4 = io.circuit_add_nand(fa2_xor, cout1);
    int fa2_g5 = io.circuit_add_nand(fa2_xor, fa2_g4);
    int fa2_g6 = io.circuit_add_nand(cout1, fa2_g4);
    int s1 = io.circuit_add_nand(fa2_g5, fa2_g6);  // S1 = XOR(XOR(A1,B1), COUT1)
    
    // Cout ng FA2
    int fa2_g7 = io.circuit_add_nand(1, 5);
    int and_a1b1 = io.circuit_add_nand(fa2_g7, fa2_g7);
    int fa2_g8 = io.circuit_add_nand(cout1, fa2_xor);
    int and_c_xor = io.circuit_add_nand(fa2_g8, fa2_g8);
    int fa2_g9 = io.circuit_add_nand(and_a1b1, and_a1b1);
    int fa2_g10 = io.circuit_add_nand(and_c_xor, and_c_xor);
    int cout2 = io.circuit_add_nand(fa2_g9, fa2_g10);
    
    // Full Adder 3
    int fa3_g1 = io.circuit_add_nand(2, 6);
    int fa3_g2 = io.circuit_add_nand(2, fa3_g1);
    int fa3_g3 = io.circuit_add_nand(6, fa3_g1);
    int fa3_xor = io.circuit_add_nand(fa3_g2, fa3_g3);
    int fa3_g4 = io.circuit_add_nand(fa3_xor, cout2);
    int fa3_g5 = io.circuit_add_nand(fa3_xor, fa3_g4);
    int fa3_g6 = io.circuit_add_nand(cout2, fa3_g4);
    int s2 = io.circuit_add_nand(fa3_g5, fa3_g6);
    
    int fa3_g7 = io.circuit_add_nand(2, 6);
    int and_a2b2 = io.circuit_add_nand(fa3_g7, fa3_g7);
    int fa3_g8 = io.circuit_add_nand(cout2, fa3_xor);
    int and_c2_xor = io.circuit_add_nand(fa3_g8, fa3_g8);
    int fa3_g9 = io.circuit_add_nand(and_a2b2, and_a2b2);
    int fa3_g10 = io.circuit_add_nand(and_c2_xor, and_c2_xor);
    int cout3 = io.circuit_add_nand(fa3_g9, fa3_g10);
    
    // Full Adder 4 (most significant)
    int fa4_g1 = io.circuit_add_nand(3, 7);
    int fa4_g2 = io.circuit_add_nand(3, fa4_g1);
    int fa4_g3 = io.circuit_add_nand(7, fa4_g1);
    int fa4_xor = io.circuit_add_nand(fa4_g2, fa4_g3);
    int fa4_g4 = io.circuit_add_nand(fa4_xor, cout3);
    int fa4_g5 = io.circuit_add_nand(fa4_xor, fa4_g4);
    int fa4_g6 = io.circuit_add_nand(cout3, fa4_g4);
    int s3 = io.circuit_add_nand(fa4_g5, fa4_g6);
    
    int fa4_g7 = io.circuit_add_nand(3, 7);
    int and_a3b3 = io.circuit_add_nand(fa4_g7, fa4_g7);
    int fa4_g8 = io.circuit_add_nand(cout3, fa4_xor);
    int and_c3_xor = io.circuit_add_nand(fa4_g8, fa4_g8);
    int fa4_g9 = io.circuit_add_nand(and_a3b3, and_a3b3);
    int fa4_g10 = io.circuit_add_nand(and_c3_xor, and_c3_xor);
    int cout4 = io.circuit_add_nand(fa4_g9, fa4_g10);
    
    // Register all outputs: S0, S1, S2, S3, COUT4
    io.add_output(s0);
    io.add_output(s1);
    io.add_output(s2);
    io.add_output(s3);
    io.add_output(cout4);
    
    std::cout << "Circuit: " << io.circuit_size() << " NAND gates\n";
    std::cout << "Outputs: S0, S1, S2, S3, COUT4\n\n";
    
    // Test all 256 combinations
    int pass = 0;
    int total = 256;
    
    for (int i = 0; i < 256; i++) {
        bool A0 = (i >> 0) & 1;
        bool A1 = (i >> 1) & 1;
        bool A2 = (i >> 2) & 1;
        bool A3 = (i >> 3) & 1;
        bool B0 = (i >> 4) & 1;
        bool B1 = (i >> 5) & 1;
        bool B2 = (i >> 6) & 1;
        bool B3 = (i >> 7) & 1;
        
        int A = A0 + 2*A1 + 4*A2 + 8*A3;
        int B = B0 + 2*B1 + 4*B2 + 8*B3;
        int expected_sum = A + B;
        
        std::vector<bool> input = {A0, A1, A2, A3, B0, B1, B2, B3};
        auto outputs = io.evaluate_multi(input);
        
        int actual_sum = outputs[0] + 2*outputs[1] + 4*outputs[2] + 8*outputs[3] + 16*outputs[4];
        
        if (actual_sum == expected_sum) {
            pass++;
        } else {
            std::cout << "  MISMATCH: A=" << A << " B=" << B 
                      << " → expected " << expected_sum 
                      << ", got " << actual_sum << "\n";
        }
    }
    
    std::cout << "\nResult: " << pass << "/" << total << " PASS\n";
    std::cout << (pass == total ? "ALL 256 COMBINATIONS PASS ✓" : "FAIL ✗") << "\n";
    
    return pass == total ? 0 : 1;
}
