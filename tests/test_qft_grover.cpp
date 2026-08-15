#include <iostream>
#include <complex>
#include <cmath>
#include <vector>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// ============ QUANTUM FOURIER TRANSFORM (3 QUBITS) ============

struct ThreeQubit {
    std::complex<double> amps[8];  // |000⟩ to |111⟩
    
    ThreeQubit() {
        for (int i = 0; i < 8; i++) amps[i] = 0;
    }
    
    // Hadamard sa qubit q
    void hadamard(int q) {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        for (int i = 0; i < 8; i++) {
            if ((i & (1 << q)) == 0) {
                int partner = i | (1 << q);
                std::complex<double> a = amps[i];
                std::complex<double> b = amps[partner];
                amps[i] = (a + b) * inv_sqrt2;
                amps[partner] = (a - b) * inv_sqrt2;
            }
        }
    }
    
    // Controlled phase rotation
    void controlled_phase(int control, int target, double angle) {
        for (int i = 0; i < 8; i++) {
            if ((i & (1 << control)) && (i & (1 << target))) {
                amps[i] *= std::exp(I * angle);
            }
        }
    }
    
    // QFT sa 3 qubits
    void qft() {
        // QFT = H(q0) → CPhase(q0,q1) → CPhase(q0,q2) → H(q1) → CPhase(q1,q2) → H(q2) → SWAP(q0,q2)
        hadamard(0);
        controlled_phase(0, 1, PI / 2.0);
        controlled_phase(0, 2, PI / 4.0);
        hadamard(1);
        controlled_phase(1, 2, PI / 2.0);
        hadamard(2);
        // SWAP(q0, q2)
        for (int i = 0; i < 8; i++) {
            int j = ((i & 4) >> 2) | (i & 2) | ((i & 1) << 2);
            if (i < j) std::swap(amps[i], amps[j]);
        }
    }
    
    // Inverse QFT
    void inverse_qft() {
        // SWAP(q0, q2)
        for (int i = 0; i < 8; i++) {
            int j = ((i & 4) >> 2) | (i & 2) | ((i & 1) << 2);
            if (i < j) std::swap(amps[i], amps[j]);
        }
        hadamard(2);
        controlled_phase(1, 2, -PI / 2.0);
        hadamard(1);
        controlled_phase(0, 2, -PI / 4.0);
        controlled_phase(0, 1, -PI / 2.0);
        hadamard(0);
    }
};

// ============ GROVER'S ALGORITHM (3 QUBITS) ============

struct Grover {
    std::complex<double> amps[8];
    
    Grover() {
        double inv_sqrt8 = 1.0 / std::sqrt(8.0);
        for (int i = 0; i < 8; i++) amps[i] = inv_sqrt8;
    }
    
    // Oracle: i-flip ang sign ng target
    void oracle(int target) {
        amps[target] = -amps[target];
    }
    
    // Diffusion: 2|ψ⟩⟨ψ| - I
    void diffusion() {
        double mean = 0;
        for (int i = 0; i < 8; i++) mean += amps[i].real();
        mean /= 8;
        
        for (int i = 0; i < 8; i++) {
            amps[i] = 2.0 * mean - amps[i];
        }
    }
    
    // Grover iteration
    void iterate(int target) {
        oracle(target);
        diffusion();
    }
    
    // Measurement
    int measure() {
        double best = -1;
        int best_idx = 0;
        for (int i = 0; i < 8; i++) {
            double prob = std::norm(amps[i]);
            if (prob > best) {
                best = prob;
                best_idx = i;
            }
        }
        return best_idx;
    }
};

int main() {
    std::cout << "QFT + GROVER'S ALGORITHM TEST\n\n";
    
    bool all_pass = true;
    
    // ============ QFT TEST ============
    std::cout << "1. QUANTUM FOURIER TRANSFORM (3 QUBITS)\n";
    
    // QFT(|000⟩) → uniform superposition
    ThreeQubit qft_state;
    qft_state.amps[0] = 1;  // |000⟩
    qft_state.qft();
    
    std::cout << "  QFT(|000⟩):\n";
    for (int i = 0; i < 8; i++) {
        std::cout << "    |" << ((i >> 2) & 1) << ((i >> 1) & 1) << (i & 1) << "⟩: "
                  << std::norm(qft_state.amps[i]) << "\n";
    }
    
    bool qft_uniform = true;
    for (int i = 0; i < 8; i++) {
        if (std::abs(std::norm(qft_state.amps[i]) - 0.125) > 0.0001) {
            qft_uniform = false;
        }
    }
    std::cout << "  " << (qft_uniform ? "✓ UNIFORM" : "❌ NOT UNIFORM") << "\n\n";
    if (!qft_uniform) all_pass = false;
    
    // Inverse QFT
    qft_state.inverse_qft();
    bool qft_inverse = (std::norm(qft_state.amps[0]) > 0.9999);
    std::cout << "  QFT⁻¹(QFT(|000⟩)) = |000⟩: " << (qft_inverse ? "✓" : "❌") << "\n\n";
    if (!qft_inverse) all_pass = false;
    
    // ============ GROVER'S TEST ============
    std::cout << "2. GROVER'S ALGORITHM (3 QUBITS)\n";
    
    // Maghanap ng target sa 8 states
    bool grover_all_pass = true;
    for (int target = 0; target < 8; target++) {
        Grover grover;
        int iterations = 2;  // Optimal para sa 3 qubits
        
        for (int i = 0; i < iterations; i++) {
            grover.iterate(target);
        }
        
        int measured = grover.measure();
        bool pass = (measured == target);
        if (!pass) grover_all_pass = false;
        
        std::cout << "  Search |" << ((target >> 2) & 1) << ((target >> 1) & 1) << (target & 1)
                  << "⟩: found |" << ((measured >> 2) & 1) << ((measured >> 1) & 1) << (measured & 1)
                  << "⟩" << (pass ? " ✓" : " ❌") << "\n";
    }
    
    std::cout << "  " << (grover_all_pass ? "✓ ALL 8 FOUND" : "❌ FAIL") << "\n\n";
    if (!grover_all_pass) all_pass = false;
    
    std::cout << "=== " << (all_pass ? "QFT + GROVER PASS" : "FAIL") << " ===\n";
    
    return 0;
}
