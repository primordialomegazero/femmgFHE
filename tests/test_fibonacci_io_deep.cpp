#include "../src/io/golden_fibonacci_io.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "FIBONACCI iO DEEP CIRCUIT TEST\n";
    std::cout << "===============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIO::FibonacciIO io(Q, 42);
    
    // ============ TEST 1: Deep NAND chain (10+ gates) ============
    std::cout << "TEST 1: DEEP NAND CHAIN (10 gates)\n";
    io.obfuscate_circuit_begin(2);
    
    // Chain: g1=NAND(a,b), g2=NAND(g1,g1), g3=NAND(g2,g2), ...
    int prev = io.circuit_add_nand(0, 1, 2000);
    for (int i = 0; i < 9; i++) {
        prev = io.circuit_add_nand(prev, prev, 2001 + i);
    }
    
    std::vector<std::vector<bool>> test_inputs = {
        {false, false},
        {false, true},
        {true, false},
        {true, true}
    };
    
    bool all_pass = true;
    for (auto& input : test_inputs) {
        bool result = io.evaluate(input);
        // After 10 NOT operations, expected = same as NAND(a,b)
        bool nand_ab = !(input[0] && input[1]);
        // 9 more NOT operations = flip 9 times
        bool expected = (9 % 2 == 0) ? nand_ab : !nand_ab;
        
        bool pass = (result == expected);
        all_pass &= pass;
        std::cout << "  Input(" << input[0] << "," << input[1] << ") = " 
                  << result << " (exp " << expected << ") " 
                  << (pass ? "✓" : "✗") << "\n";
    }
    std::cout << "  " << (all_pass ? "ALL PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ TEST 2: XOR via NAND gates ============
    std::cout << "TEST 2: XOR GATE (via NAND)\n";
    io.obfuscate_circuit_begin(2);
    
    // XOR(a,b) = NAND(NAND(a,NAND(a,b)), NAND(b,NAND(a,b)))
    int nand_ab = io.circuit_add_nand(0, 1, 3000);
    int nand_a_ab = io.circuit_add_nand(0, nand_ab, 3001);
    int nand_b_ab = io.circuit_add_nand(1, nand_ab, 3002);
    int xor_out = io.circuit_add_nand(nand_a_ab, nand_b_ab, 3003);
    
    all_pass = true;
    for (auto& input : test_inputs) {
        bool result = io.evaluate(input);
        bool expected = input[0] != input[1];  // XOR
        bool pass = (result == expected);
        all_pass &= pass;
        std::cout << "  XOR(" << input[0] << "," << input[1] << ") = " 
                  << result << " (exp " << expected << ") " 
                  << (pass ? "✓" : "✗") << "\n";
    }
    std::cout << "  " << (all_pass ? "ALL PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ TEST 3: AND + OR composite ============
    std::cout << "TEST 3: (A AND B) OR (A XOR B)\n";
    io.obfuscate_circuit_begin(2);
    
    // AND(a,b) = NOT(NAND(a,b))
    int nand1 = io.circuit_add_nand(0, 1, 4000);
    int and_ab = io.circuit_add_nand(nand1, nand1, 4001);
    
    // XOR(a,b)
    int nand_a_ab2 = io.circuit_add_nand(0, nand1, 4002);
    int nand_b_ab2 = io.circuit_add_nand(1, nand1, 4003);
    int xor_ab = io.circuit_add_nand(nand_a_ab2, nand_b_ab2, 4004);
    
    // OR(AND, XOR) = NAND(NOT(AND), NOT(XOR))
    int not_and = io.circuit_add_nand(and_ab, and_ab, 4005);
    int not_xor = io.circuit_add_nand(xor_ab, xor_ab, 4006);
    int final_out = io.circuit_add_nand(not_and, not_xor, 4007);
    
    all_pass = true;
    for (auto& input : test_inputs) {
        bool result = io.evaluate(input);
        bool and_val = input[0] && input[1];
        bool xor_val = input[0] != input[1];
        bool expected = and_val || xor_val;
        bool pass = (result == expected);
        all_pass &= pass;
        std::cout << "  (" << input[0] << "&&" << input[1] << ")||(" 
                  << input[0] << "^" << input[1] << ") = " 
                  << result << " (exp " << expected << ") " 
                  << (pass ? "✓" : "✗") << "\n";
    }
    std::cout << "  " << (all_pass ? "ALL PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ BENCHMARK ============
    std::cout << "BENCHMARK\n";
    
    // Truth table obfuscation benchmark
    auto start = std::chrono::high_resolution_clock::now();
    auto func = [](const std::vector<bool>& in) {
        return in[0] && in[1];
    };
    io.obfuscate_truth_table(func, 4, 5000);
    auto end = std::chrono::high_resolution_clock::now();
    auto obf_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  Obfuscate 16-entry truth table: " << obf_time << "ms\n";
    
    // Evaluation benchmark (1000 evaluations)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        std::vector<bool> input = {(i&1), (i&2)>>1, (i&4)>>2, (i&8)>>3};
        io.evaluate(input);
    }
    end = std::chrono::high_resolution_clock::now();
    auto eval_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double evals_per_sec = 1000.0 / (eval_time / 1000.0);
    std::cout << "  1000 evaluations: " << eval_time << "ms (" 
              << evals_per_sec << " evals/sec)\n";
    
    std::cout << "\n=== FIBONACCI iO DEEP TEST COMPLETE ===\n";
    
    return all_pass ? 0 : 1;
}
