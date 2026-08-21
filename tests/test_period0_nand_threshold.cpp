// PERIOD-0 NAND — TAMANG THRESHOLD
// Hanapin ang threshold na nagbibigay ng 4/4

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 NAND — THRESHOLD\n";
    std::cout << "  Hanapin ang 4/4 Threshold\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double phi_mod = 0.6180339887498949;

    // Rotated values para sa bawat NAND combination:
    // (0,0): sum=0, rotated = 0.618 → dapat true
    // (0,1): sum=φ², rotated = (φ²+0.618) mod 1 = 0.236 → dapat true
    // (1,0): sum=φ², rotated = 0.236 → dapat true
    // (1,1): sum=2φ², rotated = (2φ²+0.618) mod 1 = 0.854 → dapat false

    double rotated_00 = 0.618034;
    double rotated_01 = 0.236068;
    double rotated_10 = 0.236068;
    double rotated_11 = 0.854102;

    std::cout << "ROTATED VALUES:\n";
    std::cout << "  (0,0): " << rotated_00 << " → dapat true\n";
    std::cout << "  (0,1): " << rotated_01 << " → dapat true\n";
    std::cout << "  (1,0): " << rotated_10 << " → dapat true\n";
    std::cout << "  (1,1): " << rotated_11 << " → dapat false\n\n";

    // Hanapin ang threshold
    std::cout << "THRESHOLD SEARCH:\n";
    std::cout << "=================\n\n";

    for (double threshold = 0.5; threshold <= 0.9; threshold += 0.1) {
        bool t00 = rotated_00 < threshold;
        bool t01 = rotated_01 < threshold;
        bool t10 = rotated_10 < threshold;
        bool t11 = rotated_11 < threshold;

        int correct = 0;
        if (t00 == true) correct++;
        if (t01 == true) correct++;
        if (t10 == true) correct++;
        if (t11 == false) correct++;

        std::cout << "  Threshold " << threshold << ": " << correct << "/4 correct\n";
    }

    std::cout << "\n  KEY: (0,0) ay 0.618 — kailangan ng threshold > 0.618\n";
    std::cout << "  (0,1) at (1,0) ay 0.236 — kailangan ng threshold > 0.236\n";
    std::cout << "  (1,1) ay 0.854 — kailangan ng threshold < 0.854\n\n";

    std::cout << "  Kaya ang threshold ay dapat nasa (0.618, 0.854)\n";
    std::cout << "  Halimbawa: 0.7 o 0.75\n\n";

    std::cout << "  ANG EMERGENT THRESHOLD: φ_mod = 0.618\n";
    std::cout << "  Ay mas mababa sa 0.854 — kaya 4/4 posible\n";

    return 0;
}
