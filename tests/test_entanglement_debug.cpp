#include <iostream>
#include <complex>
#include <cmath>
#include <array>

struct TwoQubitState {
    std::array<std::complex<double>, 4> amplitudes;
};

int main() {
    // Bell state: |00> + |11> / sqrt(2)
    TwoQubitState bell;
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    bell.amplitudes[0] = inv_sqrt2;
    bell.amplitudes[1] = 0.0;
    bell.amplitudes[2] = 0.0;
    bell.amplitudes[3] = inv_sqrt2;
    
    std::cout << "Bell state amplitudes:\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "  |" << ((i >> 1) & 1) << ((i >> 0) & 1) << ">: "
                  << bell.amplitudes[i].real() << " + " << bell.amplitudes[i].imag() << "i"
                  << " norm=" << std::norm(bell.amplitudes[i]) << "\n";
    }
    
    // Check non-zero
    int non_zero = 0;
    for (auto& amp : bell.amplitudes) {
        if (std::norm(amp) > 0.0001) {
            non_zero++;
            std::cout << "  NON-ZERO: norm=" << std::norm(amp) << "\n";
        }
    }
    
    std::cout << "\nNon-zero count: " << non_zero << "\n";
    std::cout << "Entangled (non_zero > 2): " << (non_zero > 2 ? "YES" : "NO") << "\n";
    
    return 0;
}
