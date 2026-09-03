// ============================================
// φ-DUALITY
// Ang φ at φ⁻¹ bilang natural na separation
// para sa addition at multiplication
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-DUALITY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    // ============================================
    // TEST 1: DUALITY PROPERTIES
    // ============================================

    cout << "========================================\n";
    cout << "  DUALITY PROPERTIES\n";
    cout << "========================================\n\n";

    cout << "  φ = " << PHI << "\n";
    cout << "  φ⁻¹ = " << PHI_INV << "\n\n";

    cout << "  φ + φ⁻¹ = √5\n";
    cout << "  φ - φ⁻¹ = 1\n";
    cout << "  φ × φ⁻¹ = 1\n\n";

    // ============================================
    // TEST 2: ENCODING GAMIT ANG DUALITY
    // ============================================

    cout << "========================================\n";
    cout << "  ENCODING GAMIT ANG DUALITY\n";
    cout << "========================================\n\n";

    cout << "  I-encode ang value bilang:\n";
    cout << "  x → (x×φ, x×φ⁻¹)\n\n";

    cout << "  x | x×φ | x×φ⁻¹ | x×φ + x×φ⁻¹\n";
    cout << "  --|-----|-------|---------------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0}) {
        double mul_phi = x * PHI;
        double mul_inv = x * PHI_INV;
        double sum = mul_phi + mul_inv;
        
        cout << "  " << setw(2) << x << " | "
             << setw(5) << fixed << setprecision(3) << mul_phi << " | "
             << setw(5) << mul_inv << " | "
             << setw(9) << sum << "\n";
    }

    // ============================================
    // TEST 3: ADDITION SA DUALITY SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA DUALITY SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a×φ + a×φ⁻¹) + (b×φ + b×φ⁻¹)\n";
    cout << "  = (a+b)×φ + (a+b)×φ⁻¹\n\n";

    cout << "  a+b | Addition result | Expected | Match?\n";
    cout << "  ----|----------------|---------|--------\n";

    for (double a : {2.0, 3.0}) {
        for (double b : {2.0, 3.0}) {
            double sum_phi = (a+b) * PHI;
            double sum_inv = (a+b) * PHI_INV;
            double total = sum_phi + sum_inv;
            double expected = (a+b) * (PHI + PHI_INV);
            
            cout << "  " << a << "+" << b << " | "
                 << fixed << setprecision(4) << total << " | "
                 << setw(7) << expected << " | "
                 << (abs(total - expected) < 0.001 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 4: MULTIPLICATION SA DUALITY SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  MULTIPLICATION SA DUALITY SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a×φ)(b×φ⁻¹) = a×b×φ×φ⁻¹ = a×b\n";
    cout << "  (a×φ⁻¹)(b×φ) = a×b×φ⁻¹×φ = a×b\n\n";

    cout << "  a×b | Product result | Expected | Match?\n";
    cout << "  ----|---------------|---------|--------\n";

    for (double a : {2.0, 3.0}) {
        for (double b : {2.0, 3.0}) {
            double prod1 = (a * PHI) * (b * PHI_INV);
            double prod2 = (a * PHI_INV) * (b * PHI);
            double expected = a * b;
            
            cout << "  " << a << "×" << b << " | "
                 << fixed << setprecision(4) << prod1 << " | "
                 << setw(7) << expected << " | "
                 << (abs(prod1 - expected) < 0.001 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // KEY INSIGHT
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  Ang φ at φ⁻¹ ay natural na dual:\n";
    cout << "  - Addition: (a+b)(φ+φ⁻¹) — EvalAdd\n";
    cout << "  - Multiplication: a×b — EvalAdd sa log space\n";
    cout << "  - φ × φ⁻¹ = 1 — natural na cancellation\n\n";

    cout << "  ANG TRICK:\n";
    cout << "  Kapag i-encode natin ang value bilang\n";
    cout << "  (xφ, xφ⁻¹), ang addition at multiplication\n";
    cout << "  ay natural na naghihiwalay.\n\n";

    return 0;
}
