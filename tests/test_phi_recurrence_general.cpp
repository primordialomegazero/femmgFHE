// GENERAL RECURRENCE ANALYSIS
// x_{n+1} = x_n - x_{n-1} + K para sa iba't ibang K
// At ang coefficient B analysis

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  GENERAL RECURRENCE ANALYSIS\n";
    cout << "  Different K, B Coefficient, at Complex Form\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // ============================================
    // 1. DIFFERENT K VALUES
    // ============================================
    cout << "1. DIFFERENT K VALUES:\n";
    cout << "=======================\n\n";

    for (double K : {0.5, 1.0, phi, 2.0, 2.618, 3.0}) {
        cout << "  K = " << K << ":\n";
        
        double x_prev = 0.0;
        double x_curr = K;  // para sa non-zero start
        
        cout << "    Sequence: ";
        for (int i = 0; i < 12; i++) {
            double x_next = x_curr - x_prev + K;
            cout << x_next << " ";
            x_prev = x_curr;
            x_curr = x_next;
        }
        cout << "\n\n";
    }

    // ============================================
    // 2. COEFFICIENT B ANALYSIS
    // ============================================
    cout << "2. COEFFICIENT B = 2.089:\n";
    cout << "=========================\n\n";

    double B = (1.0 + 0.5 * phi) / 0.866;
    cout << "  B = (1 + φ/2) / sin(π/3)\n";
    cout << "  B = " << B << "\n\n";

    cout << "  φ relationships:\n";
    cout << "  B² = " << (B * B) << "\n";
    cout << "  B/φ = " << (B / phi) << "\n";
    cout << "  B·φ = " << (B * phi) << "\n";
    cout << "  B - φ = " << (B - phi) << "\n";
    cout << "  B - φ² = " << (B - phi * phi) << "\n\n";

    cout << "  Fibonacci check:\n";
    cout << "  B ≈ F(?) = " << B << "\n";
    cout << "  F(8) = 21, F(9) = 34\n";
    cout << "  2.089 ≈ 2 + 0.089 ≈ 2 + 1/11.25\n\n";

    // ============================================
    // 3. COMPLEX REPRESENTATION
    // ============================================
    cout << "3. COMPLEX REPRESENTATION:\n";
    cout << "==========================\n\n";

    complex<double> r(0.5, 0.866);  // e^{iπ/3}
    
    cout << "  r = e^{iπ/3} = " << r << "\n";
    cout << "  r⁶ = " << pow(r, 6) << " (dapat 1)\n\n";

    // General solution: x_n = C₁rⁿ + C₂r⁻ⁿ + φ
    cout << "  General solution:\n";
    cout << "  x_n = C₁·e^{inπ/3} + C₂·e^{-inπ/3} + φ\n\n";

    // ============================================
    // 4. HIGHER ORDER RECURRENCES
    // ============================================
    cout << "4. HIGHER ORDER RECURRENCES:\n";
    cout << "============================\n\n";

    // x_{n+1} = x_n - x_{n-2} + φ
    cout << "  A: x_{n+1} = x_n - x_{n-2} + φ:\n";
    double a2 = 0, a1 = 0, a0 = phi;
    cout << "    ";
    for (int i = 0; i < 10; i++) {
        double a_next = a0 - a2 + phi;
        cout << a_next << " ";
        a2 = a1;
        a1 = a0;
        a0 = a_next;
    }
    cout << "\n\n";

    // x_{n+1} = x_n - x_{n-3} + φ
    cout << "  B: x_{n+1} = x_n - x_{n-3} + φ:\n";
    double b3 = 0, b2 = 0, b1 = 0, b0 = phi;
    cout << "    ";
    for (int i = 0; i < 10; i++) {
        double b_next = b0 - b3 + phi;
        cout << b_next << " ";
        b3 = b2;
        b2 = b1;
        b1 = b0;
        b0 = b_next;
    }
    cout << "\n\n";

    // x_{n+1} = x_n - 2·x_{n-1} + φ
    cout << "  C: x_{n+1} = x_n - 2·x_{n-1} + φ:\n";
    double c1 = 0, c0 = phi;
    cout << "    ";
    for (int i = 0; i < 10; i++) {
        double c_next = c0 - 2 * c1 + phi;
        cout << c_next << " ";
        c1 = c0;
        c0 = c_next;
    }
    cout << "\n\n";

    // ============================================
    // 5. BOUNDED CHECK
    // ============================================
    cout << "5. BOUNDED CHECK:\n";
    cout << "=================\n\n";

    cout << "  Recurrence A (x_n - x_{n-2} + φ):\n";
    a2 = 0; a1 = 0; a0 = phi;
    bool bounded_A = true;
    for (int i = 0; i < 100; i++) {
        double a_next = a0 - a2 + phi;
        if (std::abs(a_next) > 100) { bounded_A = false; break; }
        a2 = a1; a1 = a0; a0 = a_next;
    }
    cout << "    Bounded: " << (bounded_A ? "YES" : "NO") << "\n\n";

    cout << "  Recurrence B (x_n - x_{n-3} + φ):\n";
    b3 = 0; b2 = 0; b1 = 0; b0 = phi;
    bool bounded_B = true;
    for (int i = 0; i < 100; i++) {
        double b_next = b0 - b3 + phi;
        if (std::abs(b_next) > 100) { bounded_B = false; break; }
        b3 = b2; b2 = b1; b1 = b0; b0 = b_next;
    }
    cout << "    Bounded: " << (bounded_B ? "YES" : "NO") << "\n\n";

    cout << "  Recurrence C (x_n - 2·x_{n-1} + φ):\n";
    c1 = 0; c0 = phi;
    bool bounded_C = true;
    for (int i = 0; i < 100; i++) {
        double c_next = c0 - 2 * c1 + phi;
        if (std::abs(c_next) > 100) { bounded_C = false; break; }
        c1 = c0; c0 = c_next;
    }
    cout << "    Bounded: " << (bounded_C ? "YES" : "NO") << "\n\n";

    // ============================================
    // 6. SUMMARY
    // ============================================
    cout << "========================================\n";
    cout << "  SUMMARY:\n";
    cout << "  - B = (1+φ/2)/sin(π/3) ay may φ connection\n";
    cout << "  - Complex form: e^{inπ/3}\n";
    cout << "  - Higher order recurrences ay may iba't ibang boundedness\n";
    cout << "  - Ang period-6 ay unique sa x_n - x_{n-1} + K\n";
    cout << "========================================\n";

    return 0;
}
