#include "../src/quantum/golden_fibonacci_quantum.h"
#include <iostream>

int main() {
    std::cout << "FUSED CLASSICAL-QUANTUM FHE TEST\n";
    std::cout << "================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    GoldenFibonacciQuantum::FusedQuantumFHE fused(Q, 42);
    
    // Test quantum gates
    fused.test_quantum_gates();
    
    // Benchmark
    std::cout << "\n";
    fused.benchmark_fused(100);
    
    return 0;
}
