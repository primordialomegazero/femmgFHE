#include <iostream>
#include <complex>
#include <cmath>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// Deep quantum gates na may complex amplitudes

struct Qubit {
    std::complex<double> amp_0;
    std::complex<double> amp_1;
    
    // Hadamard: H|0⟩ = (|0⟩+|1⟩)/√2, H|1⟩ = (|0⟩-|1⟩)/√2
    void hadamard() {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        std::complex<double> new_0 = (amp_0 + amp_1) * inv_sqrt2;
        std::complex<double> new_1 = (amp_0 - amp_1) * inv_sqrt2;
        amp_0 = new_0;
        amp_1 = new_1;
    }
    
    // Pauli-X (NOT): X|0⟩ = |1⟩, X|1⟩ = |0⟩
    void pauli_x() {
        std::swap(amp_0, amp_1);
    }
    
    // Phase gate S: S|0⟩ = |0⟩, S|1⟩ = i|1⟩
    void phase_s() {
        amp_1 *= I;
    }
    
    // T gate: T|0⟩ = |0⟩, T|1⟩ = e^(iπ/4)|1⟩
    void t_gate() {
        amp_1 *= std::exp(I * PI / 4.0);
    }
    
    // Measurement (Born rule)
    int measure() {
        double prob_0 = std::norm(amp_0);
        double rand_val = (double)rand() / RAND_MAX;
        return rand_val < prob_0 ? 0 : 1;
    }
};

// 2-qubit state para sa CNOT at entanglement
struct TwoQubit {
    std::complex<double> amp_00, amp_01, amp_10, amp_11;
    
    TwoQubit() : amp_00(1), amp_01(0), amp_10(0), amp_11(0) {}
    
    // CNOT: control=qubit0, target=qubit1
    void cnot() {
        std::swap(amp_10, amp_11);
    }
    
    // Hadamard sa qubit 0
    void hadamard_q0() {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        std::complex<double> new_00 = (amp_00 + amp_10) * inv_sqrt2;
        std::complex<double> new_01 = (amp_01 + amp_11) * inv_sqrt2;
        std::complex<double> new_10 = (amp_00 - amp_10) * inv_sqrt2;
        std::complex<double> new_11 = (amp_01 - amp_11) * inv_sqrt2;
        amp_00 = new_00; amp_01 = new_01;
        amp_10 = new_10; amp_11 = new_11;
    }
    
    // Bell state: H(q0) + CNOT
    void bell_state() {
        hadamard_q0();
        cnot();
    }
    
    // Check kung entangled
    bool is_entangled() {
        // Separable kung amp_00*amp_11 == amp_01*amp_10
        std::complex<double> product1 = amp_00 * amp_11;
        std::complex<double> product2 = amp_01 * amp_10;
        return std::abs(product1 - product2) > 0.0001;
    }
};

int main() {
    std::cout << "DEEP QUANTUM GATES TEST\n\n";
    
    bool all_pass = true;
    
    // Test 1: Hadamard (deep)
    std::cout << "1. HADAMARD:\n";
    Qubit q;
    q.amp_0 = 1; q.amp_1 = 0;  // |0⟩
    q.hadamard();
    
    double prob_0 = std::norm(q.amp_0);
    double prob_1 = std::norm(q.amp_1);
    
    std::cout << "  H|0⟩: P(0)=" << prob_0 << " P(1)=" << prob_1 << "\n";
    bool h_pass = (std::abs(prob_0 - 0.5) < 0.0001 && std::abs(prob_1 - 0.5) < 0.0001);
    std::cout << "  " << (h_pass ? "✓" : "❌") << "\n\n";
    if (!h_pass) all_pass = false;
    
    // Test 2: Double Hadamard = Identity
    std::cout << "2. H(H|0⟩) = |0⟩:\n";
    q.hadamard();  // Second Hadamard
    prob_0 = std::norm(q.amp_0);
    prob_1 = std::norm(q.amp_1);
    std::cout << "  P(0)=" << prob_0 << " P(1)=" << prob_1 << "\n";
    bool hh_pass = (prob_0 > 0.9999);
    std::cout << "  " << (hh_pass ? "✓" : "❌") << "\n\n";
    if (!hh_pass) all_pass = false;
    
    // Test 3: Pauli-X
    std::cout << "3. PAULI-X (NOT):\n";
    Qubit qx;
    qx.amp_0 = 1; qx.amp_1 = 0;
    qx.pauli_x();
    std::cout << "  X|0⟩: P(0)=" << std::norm(qx.amp_0) << " P(1)=" << std::norm(qx.amp_1) << "\n";
    bool x_pass = (std::norm(qx.amp_1) > 0.9999);
    std::cout << "  " << (x_pass ? "✓" : "❌") << "\n\n";
    if (!x_pass) all_pass = false;
    
    // Test 4: Bell State (entanglement)
    std::cout << "4. BELL STATE:\n";
    TwoQubit bell;
    bell.bell_state();
    
    std::cout << "  |00⟩: " << std::norm(bell.amp_00) << "\n";
    std::cout << "  |01⟩: " << std::norm(bell.amp_01) << "\n";
    std::cout << "  |10⟩: " << std::norm(bell.amp_10) << "\n";
    std::cout << "  |11⟩: " << std::norm(bell.amp_11) << "\n";
    
    bool bell_pass = (std::abs(std::norm(bell.amp_00) - 0.5) < 0.0001 &&
                      std::abs(std::norm(bell.amp_11) - 0.5) < 0.0001);
    std::cout << "  " << (bell_pass ? "✓" : "❌") << "\n";
    
    bool entangled = bell.is_entangled();
    std::cout << "  Entangled: " << (entangled ? "YES" : "NO") << "\n";
    std::cout << "  " << (entangled ? "✓" : "❌") << "\n\n";
    if (!bell_pass || !entangled) all_pass = false;
    
    std::cout << "=== " << (all_pass ? "DEEP QUANTUM GATES PASS" : "FAIL") << " ===\n";
    
    return 0;
}
