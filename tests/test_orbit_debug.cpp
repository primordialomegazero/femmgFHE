#include <iostream>
#include <complex>
#include <cmath>

constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

int main() {
    double golden_angle = 2.0 * PI / PHI;
    
    std::cout << "Golden angle: " << golden_angle << " radians = " 
              << golden_angle * 180.0 / PI << " degrees\n\n";
    
    // Test: TRUE encoding (positive rotation)
    double phase_true = 1.0;
    int orbit_true = 2;
    std::complex<double> val_true = std::exp(I * (phase_true + golden_angle * orbit_true));
    
    double angle_true = std::arg(val_true);
    double normalized_true = std::fmod(angle_true, golden_angle);
    
    std::cout << "TRUE encoding:\n";
    std::cout << "  Complex value: " << val_true.real() << " + " << val_true.imag() << "i\n";
    std::cout << "  Angle: " << angle_true << "\n";
    std::cout << "  Normalized (fmod): " << normalized_true << "\n";
    std::cout << "  Should be < golden_angle/2 = " << golden_angle/2 << " → TRUE\n\n";
    
    // Test: FALSE encoding (negative rotation)
    double phase_false = 2.0;
    int orbit_false = 3;
    std::complex<double> val_false = std::exp(I * (phase_false - golden_angle * orbit_false));
    
    double angle_false = std::arg(val_false);
    double normalized_false = std::fmod(angle_false, golden_angle);
    // fmod ay nagbibigay ng negative result kapag negative ang input
    if (normalized_false < 0) normalized_false += golden_angle;
    
    std::cout << "FALSE encoding:\n";
    std::cout << "  Complex value: " << val_false.real() << " + " << val_false.imag() << "i\n";
    std::cout << "  Angle: " << angle_false << "\n";
    std::cout << "  Normalized (fmod): " << normalized_false << "\n";
    std::cout << "  Should be >= golden_angle/2 = " << golden_angle/2 << " → FALSE\n\n";
    
    // Ang problema: ang std::arg ay nagbabalik ng value sa range [-π, π]
    // Kaya ang fmod ay hindi nagbibigay ng tamang normalized value
    
    // Solusyon: gamitin ang atan2 para sa full range [0, 2π)
    auto normalize_angle = [&](double angle) {
        double result = std::fmod(angle, 2.0 * PI);
        if (result < 0) result += 2.0 * PI;
        return result;
    };
    
    double norm_true = normalize_angle(angle_true);
    double norm_false = normalize_angle(angle_false);
    
    std::cout << "FIXED normalization:\n";
    std::cout << "  TRUE angle: " << norm_true << " → " 
              << (norm_true < golden_angle ? "TRUE" : "FALSE") << "\n";
    std::cout << "  FALSE angle: " << norm_false << " → " 
              << (norm_false < golden_angle ? "TRUE" : "FALSE") << "\n";
    
    return 0;
}
