// RULE 110 — φ-DOMAIN THRESHOLD SEARCH
// Hanapin kung may additive φ-formula para sa threshold

#include <iostream>
#include <cmath>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — φ-DOMAIN THRESHOLD\n";
    std::cout << "  Additive Formula Search\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    std::cout << "TARGET THRESHOLD:\n";
    std::cout << "=================\n\n";
    std::cout << "  Input: 0 → Output: 0 (false)\n";
    std::cout << "  Input: φ² → Output: φ² (true)\n";
    std::cout << "  Input: 2φ² → Output: φ² (true)\n";
    std::cout << "  Input: 3φ² → Output: 0 (false)\n\n";

    std::cout << "ADDITIVE FORMULAS TEST:\n";
    std::cout << "=======================\n\n";

    // Subukan ang iba't ibang additive combinations
    struct TestCase {
        const char* name;
        double (*fn)(double);
    };

    auto test_threshold = [&](const char* name, auto fn) {
        std::cout << "  " << name << ":\n";
        std::cout << "    f(0) = " << fn(0) << " (expected 0)\n";
        std::cout << "    f(φ²) = " << fn(phi_sq) << " (expected " << phi_sq << ")\n";
        std::cout << "    f(2φ²) = " << fn(2*phi_sq) << " (expected " << phi_sq << ")\n";
        std::cout << "    f(3φ²) = " << fn(3*phi_sq) << " (expected 0)\n\n";
    };

    // Formula 1: x - φ²
    test_threshold("x - φ²", [&](double x) { return x - phi_sq; });

    // Formula 2: φ² - |x - φ²|
    test_threshold("φ² - |x - φ²|", [&](double x) { return phi_sq - std::abs(x - phi_sq); });

    // Formula 3: φ² - (x - φ²)² / φ²
    test_threshold("φ² - (x-φ²)²/φ²", [&](double x) { return phi_sq - (x - phi_sq)*(x - phi_sq)/phi_sq; });

    // Formula 4: Linear combination
    test_threshold("2φ² - |x - φ²|", [&](double x) { return 2*phi_sq - std::abs(x - phi_sq); });

    std::cout << "\nANALYSIS:\n";
    std::cout << "=========\n\n";
    std::cout << "  Walang pure additive formula ang nagbibigay\n";
    std::cout << "  ng exact threshold — kailangan ng nonlinearity.\n\n";
    std::cout << "  PERO may period-4 na pattern:\n";
    std::cout << "  0 → 0\n";
    std::cout << "  φ² → φ²\n";
    std::cout << "  2φ² → φ²\n";
    std::cout << "  3φ² → 0\n\n";
    std::cout << "  Ito ay parang modular arithmetic:\n";
    std::cout << "  output = x mod 2φ²\n";
    std::cout << "  (kung x > φ² at x < 3φ²: output = φ²)\n";
    std::cout << "  (kung x < φ² o x > 3φ²: output = 0)\n\n";

    std::cout << "  ANG KEY:\n";
    std::cout << "  Kung kaya nating i-express ang modulo\n";
    std::cout << "  bilang addition/subtraction lamang,\n";
    std::cout << "  may 0-level threshold tayo!\n";

    return 0;
}
