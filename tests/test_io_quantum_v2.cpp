#include "../src/io/golden_fibonacci_io_v2.h"
#include "../src/quantum/golden_fibonacci_quantum_v2.h"
#include <iostream>

int main() {
    std::cout << "iO V2 + QUANTUM V2 TEST\n";
    std::cout << "=======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    // iO V2 test
    std::cout << "=== iO V2 TEST ===\n";
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    io.print_security_info();
    
    // Circuit test
    io.obfuscate_circuit_begin(2);
    int g1 = io.circuit_add_nand(0, 1);
    int g2 = io.circuit_add_nand(g1, g1);
    
    std::vector<std::vector<bool>> inputs = {
        {false,false}, {false,true}, {true,false}, {true,true}
    };
    
    for (auto& in : inputs) {
        bool result = io.evaluate(in);
        bool nand_ab = !(in[0] && in[1]);
        bool expected = !(nand_ab && nand_ab);
        std::cout << "  iO(" << in[0] << "," << in[1] << ") = " 
                  << result << " (exp " << expected << ") " 
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    // Benchmark iO
    std::cout << "\niO Benchmark:\n";
    io.benchmark_unlimited(10000);
    
    // Quantum V2 test
    std::cout << "\n=== QUANTUM V2 TEST ===\n";
    GoldenFibonacciQuantumV2::FusedQuantumFHEV2 quantum(Q, 42);
    quantum.test_all_quantum_gates();
    
    // Benchmark Quantum
    std::cout << "\nQuantum Benchmark:\n";
    quantum.benchmark_fused_v2(10000);
    
    return 0;
}
