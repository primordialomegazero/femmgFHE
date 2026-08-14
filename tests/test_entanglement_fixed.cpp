#include <iostream>
#include <complex>
#include <cmath>
#include <array>

struct TwoQubitState {
    std::array<std::complex<double>, 4> amplitudes;
};

// Tamang entanglement check gamit ang Schmidt decomposition
bool is_entangled(const TwoQubitState& qs) {
    // Para sa 2-qubit: entangled kung hindi separable
    // Separable kung: |psi> = (a|0> + b|1>) x (c|0> + d|1>)
    // = ac|00> + ad|01> + bc|10> + bd|11>
    // Kaya: amp[0]*amp[3] == amp[1]*amp[2]
    
    std::complex<double> product1 = qs.amplitudes[0] * qs.amplitudes[3];
    std::complex<double> product2 = qs.amplitudes[1] * qs.amplitudes[2];
    
    std::complex<double> diff = product1 - product2;
    double magnitude = std::abs(diff);
    
    return magnitude > 0.0001;
}

int main() {
    std::cout << "Testing entanglement detection...\n";
    
    // Bell state |00> + |11> / sqrt(2) - ENTANGLED
    TwoQubitState bell;
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    bell.amplitudes = {inv_sqrt2, 0.0, 0.0, inv_sqrt2};
    
    bool bell_entangled = is_entangled(bell);
    std::cout << "Bell state entangled: " << (bell_entangled ? "YES" : "NO") << " (expected YES)\n";
    
    // Separable state |00> - NOT entangled
    TwoQubitState separable;
    separable.amplitudes = {1.0, 0.0, 0.0, 0.0};
    
    bool sep_entangled = is_entangled(separable);
    std::cout << "|00> entangled: " << (sep_entangled ? "YES" : "NO") << " (expected NO)\n";
    
    // Another separable: (|0>+|1>)/sqrt(2) x |0> = (|00>+|10>)/sqrt(2)
    TwoQubitState separable2;
    separable2.amplitudes = {inv_sqrt2, 0.0, inv_sqrt2, 0.0};
    
    bool sep2_entangled = is_entangled(separable2);
    std::cout << "(H|0>)|0> entangled: " << (sep2_entangled ? "YES" : "NO") << " (expected NO)\n";
    
    if (bell_entangled && !sep_entangled && !sep2_entangled) {
        std::cout << "\n✅ ENTANGLEMENT DETECTION PASSED!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED\n";
        return 1;
    }
}
