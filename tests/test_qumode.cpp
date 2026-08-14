#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <array>

constexpr double PI = 3.14159265358979323846;
constexpr double HBAR = 1.054571817e-34;  // Reduced Planck constant
constexpr std::complex<double> I(0.0, 1.0);

// Qumode state: Continuous-variable quantum state
// Ginagamit ang position (x) at momentum (p) quadratures
struct QumodeState {
    // Gaussian state parameters
    double x_mean;       // Position mean
    double p_mean;       // Momentum mean
    double x_variance;   // Position variance
    double p_variance;   // Momentum variance
    double alpha;        // Coherent state amplitude
    int num_photons;     // Fock state photon number
};

// Vacuum state |0>
QumodeState vacuum_state() {
    QumodeState result;
    result.x_mean = 0.0;
    result.p_mean = 0.0;
    result.x_variance = 0.5;  // HBAR/2 para sa vacuum
    result.p_variance = 0.5;
    result.alpha = 0.0;
    result.num_photons = 0;
    return result;
}

// Coherent state |α>
QumodeState coherent_state(double alpha_real, double alpha_imag) {
    QumodeState result;
    result.alpha = std::sqrt(alpha_real * alpha_real + alpha_imag * alpha_imag);
    result.x_mean = std::sqrt(2.0) * alpha_real;
    result.p_mean = std::sqrt(2.0) * alpha_imag;
    result.x_variance = 0.5;
    result.p_variance = 0.5;
    result.num_photons = static_cast<int>(result.alpha * result.alpha);
    return result;
}

// Displacement operator D(α)
QumodeState displace(const QumodeState& qs, double dx, double dp) {
    QumodeState result = qs;
    result.x_mean += dx;
    result.p_mean += dp;
    return result;
}

// Squeezing operator S(r)
QumodeState squeeze(const QumodeState& qs, double r) {
    QumodeState result = qs;
    // Squeezing: variance reduces sa isang quadrature, lumalaki sa kabila
    result.x_variance *= std::exp(-2.0 * r);
    result.p_variance *= std::exp(2.0 * r);
    return result;
}

// Beam splitter (50:50)
std::pair<QumodeState, QumodeState> beam_splitter(const QumodeState& qs1, const QumodeState& qs2) {
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    
    QumodeState out1, out2;
    out1.x_mean = inv_sqrt2 * (qs1.x_mean + qs2.x_mean);
    out1.p_mean = inv_sqrt2 * (qs1.p_mean + qs2.p_mean);
    out1.x_variance = 0.5 * (qs1.x_variance + qs2.x_variance);
    out1.p_variance = 0.5 * (qs1.p_variance + qs2.p_variance);
    out1.alpha = std::sqrt(out1.x_mean * out1.x_mean + out1.p_mean * out1.p_mean) / std::sqrt(2.0);
    out1.num_photons = static_cast<int>(out1.alpha * out1.alpha);
    
    out2.x_mean = inv_sqrt2 * (qs1.x_mean - qs2.x_mean);
    out2.p_mean = inv_sqrt2 * (qs1.p_mean - qs2.p_mean);
    out2.x_variance = 0.5 * (qs1.x_variance + qs2.x_variance);
    out2.p_variance = 0.5 * (qs1.p_variance + qs2.p_variance);
    out2.alpha = std::sqrt(out2.x_mean * out2.x_mean + out2.p_mean * out2.p_mean) / std::sqrt(2.0);
    out2.num_photons = static_cast<int>(out2.alpha * out2.alpha);
    
    return {out1, out2};
}

// Golden ratio qumode: φ-squeezed state
QumodeState golden_qumode(double phi) {
    QumodeState result;
    // φ-squeezed: variance = 1/(2φ)
    result.x_mean = phi;
    result.p_mean = -1.0 / phi;  // ψ = -1/φ
    result.x_variance = 0.5 / phi;
    result.p_variance = 0.5 * phi;
    result.alpha = std::sqrt(phi * phi + 1.0 / (phi * phi));
    result.num_photons = static_cast<int>(result.alpha * result.alpha);
    return result;
}

// Wigner function value (para sa verification)
double wigner_function(const QumodeState& qs, double x, double p) {
    double dx = x - qs.x_mean;
    double dp = p - qs.p_mean;
    
    double exponent = -(dx * dx) / (2.0 * qs.x_variance) - (dp * dp) / (2.0 * qs.p_variance);
    double normalization = 1.0 / (2.0 * PI * std::sqrt(qs.x_variance * qs.p_variance));
    
    return normalization * std::exp(exponent);
}

void print_qumode(const QumodeState& qs, const std::string& label) {
    std::cout << label << ":\n";
    std::cout << "  x_mean: " << qs.x_mean << "\n";
    std::cout << "  p_mean: " << qs.p_mean << "\n";
    std::cout << "  x_var: " << qs.x_variance << "\n";
    std::cout << "  p_var: " << qs.p_variance << "\n";
    std::cout << "  α: " << qs.alpha << "\n";
    std::cout << "  <n>: " << qs.num_photons << "\n";
    std::cout << "  Uncertainty: Δx·Δp = " << std::sqrt(qs.x_variance * qs.p_variance) 
              << " (≥ " << 0.5 << " by Heisenberg)\n\n";
}

int main() {
    std::cout << "Testing Qumode (Continuous-Variable Quantum States)...\n\n";
    
    // Test 1: Vacuum state
    QumodeState vacuum = vacuum_state();
    print_qumode(vacuum, "Vacuum |0>");
    
    double uncertainty = std::sqrt(vacuum.x_variance * vacuum.p_variance);
    if (std::abs(uncertainty - 0.5) > 0.0001) {
        std::cout << "❌ FAILED: Vacuum hindi minimum uncertainty\n";
        return 1;
    }
    
    // Test 2: Coherent state
    QumodeState coherent = coherent_state(2.0, 0.0);
    print_qumode(coherent, "Coherent |α=2>");
    
    if (std::abs(coherent.x_mean - std::sqrt(8.0)) > 0.0001) {
        std::cout << "❌ FAILED: Coherent state x_mean mali\n";
        return 1;
    }
    
    // Test 3: Squeezed state
    QumodeState squeezed = squeeze(vacuum, 1.0);  // r=1
    print_qumode(squeezed, "Squeezed (r=1)");
    
    if (squeezed.x_variance >= squeezed.p_variance) {
        std::cout << "❌ FAILED: Hindi na-squeeze\n";
        return 1;
    }
    
    // Test 4: Beam splitter
    auto [out1, out2] = beam_splitter(coherent, vacuum);
    print_qumode(out1, "Beam splitter output 1");
    print_qumode(out2, "Beam splitter output 2");
    
    // Test 5: Golden qumode
    QumodeState golden = golden_qumode(1.6180339887498948482);
    print_qumode(golden, "Golden qumode (φ-squeezed)");
    
    double golden_uncertainty = std::sqrt(golden.x_variance * golden.p_variance);
    std::cout << "Golden qumode uncertainty: " << golden_uncertainty 
              << " (minimum: " << 0.5 << ")\n";
    
    if (std::abs(golden_uncertainty - 0.5) > 0.0001) {
        std::cout << "❌ FAILED: Golden qumode hindi minimum uncertainty\n";
        return 1;
    }
    
    // Test 6: Wigner function
    double wigner = wigner_function(vacuum, 0.0, 0.0);
    std::cout << "\nWigner(0,0) for vacuum: " << wigner 
              << " (expected " << 1.0 / PI << ")\n";
    
    if (std::abs(wigner - 1.0 / PI) > 0.0001) {
        std::cout << "❌ FAILED: Wigner function mali\n";
        return 1;
    }
    
    std::cout << "\n✅ QUMODE TESTS PASSED!\n";
    return 0;
}
