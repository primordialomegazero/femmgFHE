#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <array>

// 2-qubit state: |00>, |01>, |10>, |11>
struct TwoQubitState {
    std::array<std::complex<double>, 4> amplitudes;
};

// Hadamard sa qubit 0
TwoQubitState hadamard_q0(const TwoQubitState& qs) {
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    TwoQubitState result;
    
    // H x I: (|0>+|1>)/sqrt(2) para sa qubit 0
    result.amplitudes[0] = (qs.amplitudes[0] + qs.amplitudes[2]) * inv_sqrt2;  // |00> + |10>
    result.amplitudes[1] = (qs.amplitudes[1] + qs.amplitudes[3]) * inv_sqrt2;  // |01> + |11>
    result.amplitudes[2] = (qs.amplitudes[0] - qs.amplitudes[2]) * inv_sqrt2;  // |00> - |10>
    result.amplitudes[3] = (qs.amplitudes[1] - qs.amplitudes[3]) * inv_sqrt2;  // |01> - |11>
    
    return result;
}

// CNOT: control=q0, target=q1
TwoQubitState cnot_0_1(const TwoQubitState& qs) {
    TwoQubitState result;
    
    // CNOT matrix:
    // |00> -> |00>
    // |01> -> |01>
    // |10> -> |11>
    // |11> -> |10>
    result.amplitudes[0] = qs.amplitudes[0];
    result.amplitudes[1] = qs.amplitudes[1];
    result.amplitudes[2] = qs.amplitudes[3];
    result.amplitudes[3] = qs.amplitudes[2];
    
    return result;
}

// Bell state: H|0> on q0, then CNOT(q0, q1)
TwoQubitState create_bell_state() {
    TwoQubitState start;
    start.amplitudes = {1.0, 0.0, 0.0, 0.0};  // |00>
    
    TwoQubitState after_h = hadamard_q0(start);
    TwoQubitState bell = cnot_0_1(after_h);
    
    return bell;
}

// Check kung entangled (hindi separable)
bool is_entangled(const TwoQubitState& qs) {
    // Para sa pure state: entangled kung at least 2 amplitudes non-zero
    int non_zero = 0;
    for (auto& amp : qs.amplitudes) {
        if (std::norm(amp) > 0.0001) non_zero++;
    }
    return non_zero > 2;
}

int main() {
    std::cout << "Testing 2-Qubit Quantum Gates...\n";
    
    // Bell state
    TwoQubitState bell = create_bell_state();
    
    std::cout << "\nBell state (|00> + |11>)/sqrt(2):\n";
    std::cout << "  |00>: " << std::norm(bell.amplitudes[0]) << " (expected 0.5)\n";
    std::cout << "  |01>: " << std::norm(bell.amplitudes[1]) << " (expected 0.0)\n";
    std::cout << "  |10>: " << std::norm(bell.amplitudes[2]) << " (expected 0.0)\n";
    std::cout << "  |11>: " << std::norm(bell.amplitudes[3]) << " (expected 0.5)\n";
    
    if (std::abs(std::norm(bell.amplitudes[0]) - 0.5) > 0.0001 ||
        std::abs(std::norm(bell.amplitudes[3]) - 0.5) > 0.0001) {
        std::cout << "❌ FAILED: Hindi tama ang Bell state\n";
        return 1;
    }
    
    // Check entanglement
    bool entangled = is_entangled(bell);
    std::cout << "\nEntangled: " << (entangled ? "YES" : "NO") << " (expected YES)\n";
    
    if (!entangled) {
        std::cout << "❌ FAILED: Hindi entangled\n";
        return 1;
    }
    
    // CNOT sa Bell state
    TwoQubitState cnot_bell = cnot_0_1(bell);
    
    std::cout << "\nCNOT(Bell):\n";
    std::cout << "  |00>: " << std::norm(cnot_bell.amplitudes[0]) << "\n";
    std::cout << "  |11>: " << std::norm(cnot_bell.amplitudes[3]) << "\n";
    
    std::cout << "\n✅ 2-QUBIT QUANTUM GATES PASSED!\n";
    return 0;
}
