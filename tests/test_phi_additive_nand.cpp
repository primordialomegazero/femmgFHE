// φ-DOMAIN ADDITIVE NAND — 0 MULTIPLICATIONS?
// Subukan kung may addition-only formula para sa NAND

#include <iostream>
#include <cmath>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-DOMAIN ADDITIVE NAND SEARCH\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    // Truth table na hinahanap natin
    // "true" = φ, "false" = 0
    std::cout << "TARGET TRUTH TABLE:\n";
    std::cout << "===================\n\n";
    std::cout << "  NAND(0,0) = φ\n";
    std::cout << "  NAND(0,φ) = φ\n";
    std::cout << "  NAND(φ,0) = φ\n";
    std::cout << "  NAND(φ,φ) = 0\n\n";

    // Subukan natin ang iba't ibang addition-only formulas
    std::cout << "TESTING ADDITION-ONLY FORMULAS:\n";
    std::cout << "================================\n\n";

    auto test_formula = [&](const char* name, auto fn) {
        std::cout << "  " << name << ":\n";
        std::cout << "    NAND(0,0) = " << fn(0, 0) << "\n";
        std::cout << "    NAND(0,φ) = " << fn(0, phi) << "\n";
        std::cout << "    NAND(φ,0) = " << fn(phi, 0) << "\n";
        std::cout << "    NAND(φ,φ) = " << fn(phi, phi) << "\n\n";
    };

    // Formula 1: φ - (a + b)
    test_formula("φ - (a+b)", [&](double a, double b) { return phi - (a + b); });

    // Formula 2: φ + (a + b) - 2φ
    test_formula("φ + (a+b) - 2φ", [&](double a, double b) { return phi + (a + b) - 2*phi; });

    // Formula 3: φ - (a + b - φ)
    test_formula("φ - (a+b-φ)", [&](double a, double b) { return phi - (a + b - phi); });

    // Formula 4: 2φ - (a + b)
    test_formula("2φ - (a+b)", [&](double a, double b) { return 2*phi - (a + b); });

    // Formula 5: φ - |a - b|
    test_formula("φ - |a-b|", [&](double a, double b) { return phi - std::abs(a - b); });

    std::cout << "\nRESULT:\n";
    std::cout << "========\n\n";
    std::cout << "  Walang addition-only formula ang nagbibigay\n";
    std::cout << "  ng eksaktong NAND truth table.\n";
    std::cout << "  Ang NAND ay inherently multiplicative.\n\n";

    std::cout << "  PERO MAY PAG-ASA:\n";
    std::cout << "  Kung gagamit tayo ng φ² = φ + 1\n";
    std::cout << "  at period-2 properties, maaaring may\n";
    std::cout << "  hybrid additive-multiplicative formula.\n";

    return 0;
}
