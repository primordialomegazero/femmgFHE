// SPIRAL DEEP — HIDDEN LAYERS
// 2D Irrational Rotation + Composite Dynamics

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

int main() {
    std::cout << "========================================\n";
    std::cout << "  SPIRAL DEEP — HIDDEN LAYERS\n";
    std::cout << "  2D + Composite Dynamics\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;
    const double phi_sq = phi * phi;

    // ============================================
    // TEST 1: 2D IRRATION ROTATION
    // ============================================
    std::cout << "1. 2D IRRATION ROTATION:\n";
    std::cout << "========================\n\n";

    double x = 0.0, y = 0.0;
    std::cout << "  Step 0: x=" << x << " y=" << y << "\n";

    for (int i = 0; i < 10; i++) {
        x = std::fmod(x + phi_mod, 1.0);
        y = std::fmod(y + psi_mod, 1.0);
        std::cout << "  Step " << i+1 << ": x=" << x << " y=" << y 
                  << " | (x,y)=(" << x << "," << y << ")\n";
    }

    // ============================================
    // TEST 2: COMPOSITE ROTATION (φ + ψ)
    // ============================================
    std::cout << "\n2. COMPOSITE ROTATION (φ + ψ = 0):\n";
    std::cout << "==================================\n\n";

    x = 0.0;
    for (int i = 0; i < 10; i++) {
        x = std::fmod(x + phi_mod + psi_mod, 1.0);
        std::cout << "  Step " << i << ": x=" << x << "\n";
    }

    // ============================================
    // TEST 3: NESTED ROTATION
    // ============================================
    std::cout << "\n3. NESTED ROTATION:\n";
    std::cout << "==================\n\n";

    x = 0.0;
    y = 0.0;
    for (int i = 0; i < 10; i++) {
        x = std::fmod(x + phi_mod, 1.0);
        y = std::fmod(y + x, 1.0);
        std::cout << "  Step " << i << ": x=" << x << " y=" << y << "\n";
    }

    // ============================================
    // TEST 4: POWER ROTATION (φ^n)
    // ============================================
    std::cout << "\n4. POWER ROTATION (φ^n):\n";
    std::cout << "=========================\n\n";

    x = 0.0;
    for (int i = 1; i <= 10; i++) {
        double step = std::pow(phi_mod, i);
        x = std::fmod(x + step, 1.0);
        std::cout << "  Step " << i << ": step=" << step << " x=" << x << "\n";
    }

    // ============================================
    // TEST 5: GOLDEN ANGLE (137.5°)
    // ============================================
    std::cout << "\n5. GOLDEN ANGLE (137.5°):\n";
    std::cout << "=========================\n\n";

    const double golden_angle = 2.399963229728653; // 137.5° in radians
    x = 0.0;
    for (int i = 0; i < 10; i++) {
        x = std::fmod(x + golden_angle, 2 * M_PI);
        std::cout << "  Step " << i << ": θ=" << x << " (" 
                  << x * 180 / M_PI << "°)\n";
    }

    return 0;
}
