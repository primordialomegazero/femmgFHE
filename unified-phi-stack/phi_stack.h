#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

constexpr double PHI   = 1.6180339887498948482;
constexpr double PSI   = -0.6180339887498948482;
constexpr double PHI_SQ = PHI * PHI;
constexpr double PI    = 3.14159265358979323846;

static_assert(std::abs(PHI + PSI - 1.0) < 1e-10, "phi + psi must equal 1");
static_assert(std::abs(PHI * PSI + 1.0) < 1e-10, "phi * psi must equal -1");

// [THEOREM 2] φ(a,b) = a + b·φ — Active projection. See docs/FORMAL_PROOFS.md §2
inline double phi_project(double a, double b) {
    return a + b * PHI;
}

inline double psi_project(double a, double b) {
    return a + b * PSI;
}

inline double dualgate_a(double phi_val, double psi_val) {
    double b = (phi_val - psi_val) / (PHI - PSI);
    return phi_val - b * PHI;
}

inline double dualgate_b(double phi_val, double psi_val) {
    return (phi_val - psi_val) / (PHI - PSI);
}

inline int fibonacci(int n) {
    if (n <= 0) return 1;
    if (n == 1) return 2;
    int a = 1, b = 2;
    for (int i = 2; i <= n; i++) {
        int c = a + b;
        a = b;
        b = c;
    }
    return b;
}

inline double cassini_invariant(int n) {
    int fn_plus_1 = fibonacci(n + 1);
    int fn_minus_1 = fibonacci(n - 1);
    int fn = fibonacci(n);
    return std::abs((double)(fn_plus_1 * fn_minus_1 - fn * fn));
}

inline double fibonacci_anchor(int n, double seed) {
    int fib = fibonacci(n);
    double x = seed * fib * PHI;
    x = x - std::floor(x);
    return x;
}

constexpr double GOLDEN_ANGLE_RAD = 2.399963229728653;

inline double phi_rotate(double value, int round = 1) {
    double angle = GOLDEN_ANGLE_RAD * round;
    double c = std::cos(angle);
    double s = std::sin(angle);
    return value * c + (1.0 - value) * s;
}

inline double commutative_sum(const std::vector<double>& values) {
    double sum = 0;
    for (auto v : values) sum += v;
    return sum;
}

inline double commutative_product(const std::vector<double>& values) {
    double prod = 1.0;
    for (auto v : values) prod *= (v + 0.0001);
    return prod;
}

inline double commutative_harmonic(const std::vector<double>& values) {
    double sum = 0;
    for (auto v : values) sum += 1.0 / (v + 0.001);
    return values.size() / sum;
}

inline double commutative_geometric(const std::vector<double>& values) {
    double sum = 0;
    for (auto v : values) sum += std::sqrt(v + 0.0001);
    return sum / values.size();
}

inline double commutative_rms(const std::vector<double>& values) {
    double sum_sq = 0;
    for (auto v : values) sum_sq += v * v;
    return std::sqrt(sum_sq / values.size());
}

inline double commutative_reconstruct(const std::vector<double>& values) {
    double n = values.size();
    double sum_all = commutative_sum(values);
    double prod_all = commutative_product(values);
    double harm = commutative_harmonic(values);
    double rms = commutative_rms(values);
    return sum_all / n * 0.35 + 
           std::pow(prod_all, 1.0 / n) * 0.25 +
           harm * 0.25 +
           rms * 0.15;
}

inline void dualgate_superpose(double phi_A, double psi_A,
                                double phi_B, double psi_B,
                                double& phi_out, double& psi_out,
                                double master_seed = 0.5) {
    double mixed_phi = phi_A * PHI + phi_B * PSI + psi_A * PSI + psi_B * PHI;
    double mixed_psi = psi_A * PHI + psi_B * PSI + phi_A * PSI + phi_B * PHI;
    mixed_phi = mixed_phi - std::floor(mixed_phi);
    mixed_psi = mixed_psi - std::floor(mixed_psi);
    phi_out = mixed_phi * master_seed * PHI;
    psi_out = mixed_psi * master_seed * PSI;
    phi_out = phi_out - std::floor(phi_out);
    psi_out = psi_out - std::floor(psi_out);
}

inline bool verify_phi_identities() {
    if (std::abs(PHI + PSI - 1.0) > 1e-10) return false;
    if (std::abs(PHI * PSI + 1.0) > 1e-10) return false;
    if (std::abs(PHI * PHI - (PHI + 1.0)) > 1e-10) return false;
    for (int n = 2; n < 20; n++) {
        if (std::abs(cassini_invariant(n) - 1.0) > 0.01) return false;
    }
    for (double a = 0; a < 1.0; a += 0.1) {
        for (double b = 0; b < 1.0; b += 0.1) {
            double phi = phi_project(a, b);
            double psi = psi_project(a, b);
            double a_rec = dualgate_a(phi, psi);
            double b_rec = dualgate_b(phi, psi);
            if (std::abs(a - a_rec) > 0.001 || std::abs(b - b_rec) > 0.001) return false;
        }
    }
    return true;
}

inline std::string phi_stack_status() {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(10);
    ss << "Unified Phi Stack v1.0\n";
    ss << "  phi = " << PHI << "\n";
    ss << "  psi = " << PSI << "\n";
    ss << "  phi + psi = " << (PHI + PSI) << " (expect 1.0)\n";
    ss << "  phi * psi = " << (PHI * PSI) << " (expect -1.0)\n";
    ss << "  Identities: " << (verify_phi_identities() ? "ALL PASS" : "FAIL") << "\n";
    return ss.str();
}
