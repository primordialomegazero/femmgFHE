#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <algorithm>
#include <array>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// 3-qubit Grover's Algorithm
// Search sa 8 elements (N=8), may 1 solution
struct ThreeQubitState {
    std::array<std::complex<double>, 8> amplitudes;  // |000> to |111>
};

// Oracle: i-flip ang sign ng solution state
void oracle(ThreeQubitState& qs, int solution) {
    qs.amplitudes[solution] = -qs.amplitudes[solution];
}

// Diffusion operator (inversion about mean)
void diffusion(ThreeQubitState& qs) {
    double N = 8.0;
    std::complex<double> mean = 0.0;
    
    for (auto& amp : qs.amplitudes) {
        mean += amp;
    }
    mean /= N;
    
    for (auto& amp : qs.amplitudes) {
        amp = 2.0 * mean - amp;
    }
}

// Grover iteration = Oracle + Diffusion
void grover_iteration(ThreeQubitState& qs, int solution) {
    oracle(qs, solution);
    diffusion(qs);
}

// Initial state: uniform superposition
ThreeQubitState initialize() {
    ThreeQubitState result;
    double inv_sqrt8 = 1.0 / std::sqrt(8.0);
    for (int i = 0; i < 8; i++) {
        result.amplitudes[i] = inv_sqrt8;
    }
    return result;
}

// Measure: probability distribution
std::vector<double> measure(const ThreeQubitState& qs) {
    std::vector<double> probs(8);
    for (int i = 0; i < 8; i++) {
        probs[i] = std::norm(qs.amplitudes[i]);
    }
    return probs;
}

int main() {
    std::cout << "Testing Grover's Algorithm (3 qubits, N=8)...\n\n";
    
    // Test para sa bawat posibleng solution
    for (int solution = 0; solution < 8; solution++) {
        ThreeQubitState state = initialize();
        
        // Optimal number of iterations: floor(pi/4 * sqrt(N)) = 2 para sa N=8
        int iterations = 2;
        for (int i = 0; i < iterations; i++) {
            grover_iteration(state, solution);
        }
        
        // Measure
        auto probs = measure(state);
        int measured = std::max_element(probs.begin(), probs.end()) - probs.begin();
        double max_prob = probs[measured];
        
        std::cout << "Solution " << solution << " (binary: ";
        for (int bit = 2; bit >= 0; bit--) std::cout << ((solution >> bit) & 1);
        std::cout << "): measured=" << measured << " prob=" << max_prob << "\n";
        
        if (measured != solution) {
            std::cout << "  ❌ FAILED: Na-measure ang maling state\n";
            return 1;
        }
        
        if (max_prob < 0.9) {  // Grover's: ~94.5% para sa N=8, 2 iterations
            std::cout << "  ❌ FAILED: Probability masyadong mababa\n";
            return 1;
        }
    }
    
    std::cout << "\n";
    
    // Ipakita ang probability distribution para sa isang example
    ThreeQubitState demo = initialize();
    for (int i = 0; i < 2; i++) {
        grover_iteration(demo, 5);  // Search for |101>
    }
    
    auto probs = measure(demo);
    std::cout << "Probability distribution (searching |101>):\n";
    for (int i = 0; i < 8; i++) {
        std::cout << "  |";
        for (int bit = 2; bit >= 0; bit--) std::cout << ((i >> bit) & 1);
        std::cout << ">: " << probs[i] * 100 << "%\n";
    }
    
    std::cout << "\n✅ GROVER'S ALGORITHM TEST PASSED (8/8 solutions found)!\n";
    return 0;
}
