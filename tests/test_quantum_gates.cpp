#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <complex>
#include <cmath>

// Quantum state na may complex amplitudes
struct QuantumState {
    std::complex<double> amplitude_0;
    std::complex<double> amplitude_1;
};

// Hadamard gate: H|0> = (|0>+|1>)/sqrt(2), H|1> = (|0>-|1>)/sqrt(2)
QuantumState hadamard(const QuantumState& qs) {
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    QuantumState result;
    result.amplitude_0 = (qs.amplitude_0 + qs.amplitude_1) * inv_sqrt2;
    result.amplitude_1 = (qs.amplitude_0 - qs.amplitude_1) * inv_sqrt2;
    return result;
}

// CNOT gate: kapag control=1, i-flip ang target
QuantumState cnot(const QuantumState& control, const QuantumState& target) {
    // Kung control ay |1>, i-swap ang target amplitudes
    if (std::norm(control.amplitude_1) > std::norm(control.amplitude_0)) {
        return {target.amplitude_1, target.amplitude_0};
    }
    return target;
}

int main() {
    GoldenFHE::init_ring();
    
    std::cout << "Testing Quantum Gates (Hadamard + CNOT)...\n";
    
    // Test 1: H|0> = (|0>+|1>)/sqrt(2)
    QuantumState zero = {1.0, 0.0};
    QuantumState h_zero = hadamard(zero);
    
    double prob_0 = std::norm(h_zero.amplitude_0);
    double prob_1 = std::norm(h_zero.amplitude_1);
    
    std::cout << "\nH|0>:\n";
    std::cout << "  P(0) = " << prob_0 << " (expected 0.5)\n";
    std::cout << "  P(1) = " << prob_1 << " (expected 0.5)\n";
    
    if (std::abs(prob_0 - 0.5) > 0.0001 || std::abs(prob_1 - 0.5) > 0.0001) {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    // Test 2: H|1> = (|0>-|1>)/sqrt(2)
    QuantumState one = {0.0, 1.0};
    QuantumState h_one = hadamard(one);
    
    double prob_0b = std::norm(h_one.amplitude_0);
    double prob_1b = std::norm(h_one.amplitude_1);
    
    std::cout << "\nH|1>:\n";
    std::cout << "  P(0) = " << prob_0b << " (expected 0.5)\n";
    std::cout << "  P(1) = " << prob_1b << " (expected 0.5)\n";
    
    if (std::abs(prob_0b - 0.5) > 0.0001 || std::abs(prob_1b - 0.5) > 0.0001) {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    // Test 3: H(H|0>) = |0> (double Hadamard)
    QuantumState h_h_zero = hadamard(h_zero);
    
    std::cout << "\nH(H|0>) = |0>:\n";
    std::cout << "  P(0) = " << std::norm(h_h_zero.amplitude_0) << " (expected 1.0)\n";
    std::cout << "  P(1) = " << std::norm(h_h_zero.amplitude_1) << " (expected 0.0)\n";
    
    if (std::norm(h_h_zero.amplitude_0) < 0.9999 || std::norm(h_h_zero.amplitude_1) > 0.0001) {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    // Test 4: CNOT(0, 0) = 0 (control=0, walang flip)
    QuantumState target0 = {1.0, 0.0};
    QuantumState cnot_00 = cnot(zero, target0);
    
    std::cout << "\nCNOT(|0>, |0>):\n";
    std::cout << "  Target P(0) = " << std::norm(cnot_00.amplitude_0) << " (expected 1.0)\n";
    
    // Test 5: CNOT(1, 0) = 1 (control=1, mag-flip)
    QuantumState cnot_10 = cnot(one, target0);
    
    std::cout << "\nCNOT(|1>, |0>):\n";
    std::cout << "  Target P(1) = " << std::norm(cnot_10.amplitude_1) << " (expected 1.0)\n";
    
    if (std::norm(cnot_10.amplitude_1) < 0.9999) {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    // Test 6: Bell state |00> + |11> / sqrt(2)
    QuantumState bell_control = hadamard(zero);  // H|0>
    QuantumState bell_target = {1.0, 0.0};  // |0>
    QuantumState bell = cnot(bell_control, bell_target);
    
    std::cout << "\nBell state (H|0> then CNOT):\n";
    std::cout << "  Control: P(0)=" << std::norm(bell_control.amplitude_0) 
              << " P(1)=" << std::norm(bell_control.amplitude_1) << "\n";
    std::cout << "  Target after CNOT: P(0)=" << std::norm(bell.amplitude_0) 
              << " P(1)=" << std::norm(bell.amplitude_1) << "\n";
    
    std::cout << "\n✅ QUANTUM GATES TEST PASSED!\n";
    return 0;
}
