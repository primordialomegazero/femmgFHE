#include <iostream>
#include <vector>
#include <cmath>
#include <random>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

int main() {
    std::cout << "Golden iO Debug: PHI at PSI\n\n";
    
    std::cout << "PHI = " << PHI << "\n";
    std::cout << "PSI = " << PSI << "\n";
    std::cout << "PHI * PSI = " << PHI * PSI << "\n";
    std::cout << "PHI + PSI = " << PHI + PSI << "\n\n";
    
    // Test encoding para sa TRUE at FALSE
    std::cout << "TRUE encoding (PHI):\n";
    double true_val = PHI;
    std::cout << "  value = " << true_val << "\n";
    std::cout << "  value * PHI = " << true_val * PHI << "\n";
    std::cout << "  value * PSI = " << true_val * PSI << "\n\n";
    
    std::cout << "FALSE encoding (PSI):\n";
    double false_val = PSI;
    std::cout << "  value = " << false_val << "\n";
    std::cout << "  value * PHI = " << false_val * PHI << "\n";
    std::cout << "  value * PSI = " << false_val * PSI << "\n\n";
    
    // Ang problema: paano natin ma-distinguish ang TRUE sa FALSE?
    // TRUE: value = PHI > 0
    // FALSE: value = PSI < 0
    // Kaya ang test ay: value > 0 ? TRUE : FALSE
    
    std::cout << "Simpleng test:\n";
    std::cout << "  TRUE (PHI=" << PHI << "): " << (PHI > 0 ? "TRUE" : "FALSE") << "\n";
    std::cout << "  FALSE (PSI=" << PSI << "): " << (PSI > 0 ? "TRUE" : "FALSE") << "\n\n";
    
    // I-test ang fgg_multilinear
    auto fgg = [](double v, int level) {
        double result = v;
        for (int i = 0; i < level; i++) {
            result *= (i % 2 == 0 ? PHI * PSI : PSI * PHI);
        }
        return result;
    };
    
    std::cout << "fgg_multilinear(PHI, 1) = " << fgg(PHI, 1) << "\n";
    std::cout << "fgg_multilinear(PSI, 1) = " << fgg(PSI, 1) << "\n";
    std::cout << "fgg_multilinear(PHI, 2) = " << fgg(PHI, 2) << "\n";
    std::cout << "fgg_multilinear(PSI, 2) = " << fgg(PSI, 2) << "\n";
    
    return 0;
}
