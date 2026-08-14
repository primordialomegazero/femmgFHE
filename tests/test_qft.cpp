#include <iostream>
#include <complex>
#include <cmath>
#include <array>
#include <vector>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// 2-qubit Quantum Fourier Transform
struct TwoQubitState {
    std::array<std::complex<double>, 4> amplitudes;
};

// Hadamard sa qubit 0
TwoQubitState hadamard_q0(const TwoQubitState& qs) {
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    TwoQubitState result;
    
    result.amplitudes[0] = (qs.amplitudes[0] + qs.amplitudes[2]) * inv_sqrt2;
    result.amplitudes[1] = (qs.amplitudes[1] + qs.amplitudes[3]) * inv_sqrt2;
    result.amplitudes[2] = (qs.amplitudes[0] - qs.amplitudes[2]) * inv_sqrt2;
    result.amplitudes[3] = (qs.amplitudes[1] - qs.amplitudes[3]) * inv_sqrt2;
    
    return result;
}

// Hadamard sa qubit 1
TwoQubitState hadamard_q1(const TwoQubitState& qs) {
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    TwoQubitState result;
    
    result.amplitudes[0] = (qs.amplitudes[0] + qs.amplitudes[1]) * inv_sqrt2;
    result.amplitudes[1] = (qs.amplitudes[0] - qs.amplitudes[1]) * inv_sqrt2;
    result.amplitudes[2] = (qs.amplitudes[2] + qs.amplitudes[3]) * inv_sqrt2;
    result.amplitudes[3] = (qs.amplitudes[2] - qs.amplitudes[3]) * inv_sqrt2;
    
    return result;
}

// Controlled phase rotation R_k sa qubit 1 (target) na may control qubit 0
TwoQubitState controlled_phase(const TwoQubitState& qs, int k) {
    TwoQubitState result = qs;
    
    // R_k = diag(1, 1, 1, e^(2*pi*i / 2^k))
    // Kapag control=1 at target=1, i-rotate ang phase
    double phase = 2.0 * PI / std::pow(2.0, k);
    std::complex<double> rotation = std::exp(I * phase);
    
    result.amplitudes[3] *= rotation;  // |11> component
    
    return result;
}

// 2-qubit QFT
TwoQubitState qft_2qubit(const TwoQubitState& qs) {
    TwoQubitState result = qs;
    
    // QFT = H(q0) -> Controlled-R2(q0->q1) -> H(q1) -> SWAP
    result = hadamard_q0(result);
    result = controlled_phase(result, 2);  // R_2
    result = hadamard_q1(result);
    
    // SWAP qubits
    std::swap(result.amplitudes[1], result.amplitudes[2]);
    
    return result;
}

// Inverse QFT para sa verification
TwoQubitState inverse_qft_2qubit(const TwoQubitState& qs) {
    TwoQubitState result = qs;
    
    // SWAP back
    std::swap(result.amplitudes[1], result.amplitudes[2]);
    
    // H(q1) -> Controlled-R2+ -> H(q0)
    result = hadamard_q1(result);
    result = controlled_phase(result, -2);  // R_-2
    result = hadamard_q0(result);
    
    return result;
}

void print_state(const TwoQubitState& qs, const std::string& label) {
    std::cout << label << ":\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "  |" << ((i >> 1) & 1) << ((i >> 0) & 1) << ">: "
                  << qs.amplitudes[i].real() << " + " << qs.amplitudes[i].imag() << "i"
                  << " (norm=" << std::norm(qs.amplitudes[i]) << ")\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "Testing 2-Qubit Quantum Fourier Transform...\n\n";
    
    // Test 1: QFT(|00>)
    TwoQubitState zero;
    zero.amplitudes = {1.0, 0.0, 0.0, 0.0};
    
    TwoQubitState qft_zero = qft_2qubit(zero);
    print_state(qft_zero, "QFT|00>");
    
    // Expected: uniform superposition
    double expected_norm = 0.25;
    bool passed = true;
    for (int i = 0; i < 4; i++) {
        if (std::abs(std::norm(qft_zero.amplitudes[i]) - expected_norm) > 0.0001) {
            passed = false;
            break;
        }
    }
    
    if (!passed) {
        std::cout << "❌ FAILED: QFT|00> hindi uniform\n";
        return 1;
    }
    
    // Test 2: Inverse QFT(QFT|00>) = |00>
    TwoQubitState recovered = inverse_qft_2qubit(qft_zero);
    print_state(recovered, "QFT^-1(QFT|00>)");
    
    if (std::norm(recovered.amplitudes[0]) < 0.9999) {
        std::cout << "❌ FAILED: Hindi bumalik sa |00>\n";
        return 1;
    }
    
    // Test 3: QFT(|01>)
    TwoQubitState one;
    one.amplitudes = {0.0, 1.0, 0.0, 0.0};
    
    TwoQubitState qft_one = qft_2qubit(one);
    print_state(qft_one, "QFT|01>");
    
    // Expected: |00> + i|01> - |10> - i|11> / 2
    if (std::abs(std::norm(qft_one.amplitudes[0]) - 0.25) > 0.0001 ||
        std::abs(std::norm(qft_one.amplitudes[3]) - 0.25) > 0.0001) {
        std::cout << "❌ FAILED: QFT|01> hindi tama\n";
        return 1;
    }
    
    std::cout << "✅ 2-QUBIT QFT TEST PASSED!\n";
    return 0;
}
