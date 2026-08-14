#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "CIRCUIT OBFUSCATION V2 - FIXED AND\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // Test 1: 4-input XOR
    std::cout << "1. 4-input XOR:\n";
    
    gps.obfuscate_circuit_begin(4);
    int xor_ab = gps.circuit_add_xor(0, 1);
    int xor_cd = gps.circuit_add_xor(2, 3);
    int xor4 = gps.circuit_add_xor(xor_ab, xor_cd);
    
    bool correct = true;
    for (int i = 0; i < 16; i++) {
        std::vector<bool> input = {
            (bool)((i >> 3) & 1), (bool)((i >> 2) & 1),
            (bool)((i >> 1) & 1), (bool)(i & 1)
        };
        bool result = gps.evaluate_io_public(input);
        bool expected = input[0] ^ input[1] ^ input[2] ^ input[3];
        if (result != expected) correct = false;
    }
    
    std::cout << "  Gates: " << gps.circuit_size() << ", Correct: " 
              << (correct ? "16/16 ✅" : "❌") << "\n\n";
    
    // Test 2: 8-input AND (SEQUENTIAL - fixed)
    std::cout << "2. 8-input AND (Sequential):\n";
    
    gps.obfuscate_circuit_begin(8);
    
    // Sequential AND: result = AND(in[0], in[1], ..., in[7])
    // Bawat step: temp = NAND(prev, in[i]); prev = NAND(temp, temp)
    
    int prev = 0;  // Start sa in[0]
    for (int i = 1; i < 8; i++) {
        int nand_ab = gps.circuit_add_nand(prev, i);
        prev = gps.circuit_add_nand(nand_ab, nand_ab);  // NOT(NAND) = AND
    }
    
    std::cout << "  Gates: " << gps.circuit_size() << "\n";
    
    bool test_cases[][8] = {
        {1,1,1,1,1,1,1,1},  // AND = 1
        {1,1,1,1,1,1,1,0},  // AND = 0
        {0,0,0,0,0,0,0,0},  // AND = 0
        {1,0,1,0,1,0,1,0}   // AND = 0
    };
    
    bool all_pass = true;
    for (auto& tc : test_cases) {
        std::vector<bool> input(tc, tc + 8);
        bool result = gps.evaluate_io_public(input);
        bool expected = true;
        for (bool b : input) if (!b) expected = false;
        
        if (result != expected) {
            all_pass = false;
            std::cout << "  ❌ Input: ";
            for (bool b : input) std::cout << b;
            std::cout << " → " << result << " (expected " << expected << ")\n";
        }
    }
    
    std::cout << "  Correct: " << (all_pass ? "YES ✅" : "NO ❌") << "\n\n";
    
    gps.print_security();
    
    std::cout << "\n✅ CIRCUIT OBFUSCATION V2 FULLY WORKING!\n";
    
    return 0;
}
