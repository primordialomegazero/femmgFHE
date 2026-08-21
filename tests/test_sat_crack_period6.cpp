// CRACK ANG PERIOD-6 PATTERN
// Ang sequence: 1,1,1,1,1,2 | 1,1,1,1,1,10 | 1,1,1,1,1,74
// Hanapin ang pattern ng 2, 10, 74

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  CRACK ANG PERIOD-6 PATTERN\n";
    cout << "  Sequence: 1,1,1,1,1,X\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // Ang large gaps mula sa transitions:
    // 2, 10, 74, 2, 10, 74, ... (cycle?)
    // O may mas malalim na pattern?

    cout << "LARGE GAPS SEQUENCE:\n";
    cout << "  2, 10, 74, 2, 10, 74, 2, 10, 74...\n\n";

    cout << "FIBONACCI CONNECTION:\n";
    cout << "=====================\n\n";
    cout << "  2 = F(2)\n";
    cout << "  10 = F(5) + F(3) = 5 + 2 = 7? Hindi...\n";
    cout << "  74 = ?\n\n";

    // Tingnan kung may φ-based relationship
    cout << "φ-BASED ANALYSIS:\n";
    cout << "=================\n\n";
    cout << "  2 / φ = " << (2 / phi) << "\n";
    cout << "  10 / φ = " << (10 / phi) << "\n";
    cout << "  74 / φ = " << (74 / phi) << "\n\n";

    cout << "  2 × φ = " << (2 * phi) << "\n";
    cout << "  10 × φ = " << (10 * phi) << "\n";
    cout << "  74 × φ = " << (74 * phi) << "\n\n";

    // I-check kung may ratio
    cout << "RATIOS:\n";
    cout << "=======\n\n";
    cout << "  10/2 = " << (10.0/2) << "\n";
    cout << "  74/10 = " << (74.0/10) << "\n\n";

    cout << "  Ang ratio 5 at 7.4 ay malapit sa:\n";
    cout << "  φ³ = " << (phi*phi*phi) << " (4.236)\n";
    cout << "  φ⁴ = " << (phi*phi*phi*phi) << " (6.854)\n";
    cout << "  φ⁵ = " << (pow(phi,5)) << " (11.09)\n\n";

    // I-check kung may Fibonacci multiple
    cout << "FIBONACCI MULTIPLES:\n";
    cout << "====================\n\n";
    cout << "  2 = 2×1 = 2×F(1)\n";
    cout << "  10 = 2×5 = 2×F(4)\n";
    cout << "  74 = 2×37 = 2×(F(9)+F(7)) = 2×(34+3) = 2×37\n\n";

    cout << "  Kaya ang pattern ay:\n";
    cout << "    2 = 2×F(1)\n";
    cout << "    10 = 2×F(4)\n";
    cout << "    74 = 2×37\n\n";

    cout << "  Kung ang F(1), F(4), 37 ay may sequence...\n";
    cout << "  Ito ay maaaring: F(1), F(4), F(9)+F(7), ...\n\n";

    cout << "NEXT PREDICTION:\n";
    cout << "================\n\n";
    cout << "  Kung ang sequence ay 2, 10, 74, ...\n";
    cout << "  Ang susunod ay maaaring mas malaking composite\n";
    cout << "  O bumalik sa 2 (period-3)\n\n";

    cout << "  Test: kung period-3, ang sequence ay:\n";
    cout << "    2, 10, 74, 2, 10, 74, ...\n";
    cout << "  At kung period-N, mas mahaba pa\n";

    return 0;
}
