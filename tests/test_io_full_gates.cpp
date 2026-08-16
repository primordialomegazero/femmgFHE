// iO FULL GATES TEST — Lahat ng gates sa circuit mode
#include "../src/io/golden_fibonacci_io_v2.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "iO FULL GATES TEST\n";
    std::cout << "==================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    int total = 0, passed = 0;
    
    // ============ NAND ============
    std::cout << "NAND:\n";
    io.obfuscate_circuit_begin(2);
    int n1 = io.circuit_add_nand(0, 1);
    io.add_output(n1);
    
    total += 4;
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1, b = i & 1;
        bool result = io.evaluate({a, b});
        bool expected = !(a && b);
        passed += (result == expected);
        std::cout << "  NAND(" << a << "," << b << ") = " << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    std::cout << "\n";
    
    // ============ XOR ============
    std::cout << "XOR:\n";
    io.obfuscate_circuit_begin(2);
    int x1 = io.circuit_add_nand(0, 1);
    int x2 = io.circuit_add_nand(0, x1);
    int x3 = io.circuit_add_nand(1, x1);
    int x4 = io.circuit_add_nand(x2, x3);
    io.add_output(x4);
    
    total += 4;
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1, b = i & 1;
        bool result = io.evaluate({a, b});
        bool expected = a != b;
        passed += (result == expected);
        std::cout << "  XOR(" << a << "," << b << ") = " << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    std::cout << "\n";
    
    // ============ AND ============
    std::cout << "AND:\n";
    io.obfuscate_circuit_begin(2);
    int a1 = io.circuit_add_nand(0, 1);
    int a2 = io.circuit_add_nand(a1, a1);
    io.add_output(a2);
    
    total += 4;
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1, b = i & 1;
        bool result = io.evaluate({a, b});
        bool expected = a && b;
        passed += (result == expected);
        std::cout << "  AND(" << a << "," << b << ") = " << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    std::cout << "\n";
    
    // ============ OR ============
    std::cout << "OR:\n";
    io.obfuscate_circuit_begin(2);
    int o1 = io.circuit_add_nand(0, 0);
    int o2 = io.circuit_add_nand(1, 1);
    int o3 = io.circuit_add_nand(o1, o2);
    io.add_output(o3);
    
    total += 4;
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1, b = i & 1;
        bool result = io.evaluate({a, b});
        bool expected = a || b;
        passed += (result == expected);
        std::cout << "  OR(" << a << "," << b << ") = " << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    std::cout << "\nRESULT: " << passed << "/" << total << " PASS\n";
    return passed == total ? 0 : 1;
}
