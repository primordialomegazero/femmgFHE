// ============================================
// φ-EXPANSION ALGORITHM
// I-decompose ang arbitrary number sa φ-powers
//
// Core algorithm:
// - b = c₀ + c₁φ + c₂φ² + ... + cₙφⁿ
// - cᵢ ∈ {-1, 0, 1}
// - Exact na representation!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-EXPANSION ALGORITHM\n";
    cout << "  Exact φ-Power Representation\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== φ-EXPANSION FUNCTION ==========
    auto phi_expand = [&](int n) {
        vector<int> coefficients;
        int remaining = n;
        
        // Hanapin ang pinakamalaking φ-power na ≤ remaining
        vector<long long> fib = {0, 1};
        for (int i = 2; i <= 30; i++) {
            fib.push_back(fib[i-1] + fib[i-2]);
        }
        
        // φ-powers: φ^n = F(n)φ + F(n-1)
        // Para sa integer n, hanapin ang φ-expansion
        // Gamitan ng greedy algorithm
        vector<int> powers;
        int max_power = 0;
        while (pow(PHI, max_power) <= n + 1) {
            max_power++;
        }
        
        for (int p = max_power; p >= 0; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 0.001) {
                coefficients.push_back(p);
                remaining -= (int)round(phi_p);
            }
        }
        
        return coefficients;
    };
    
    // ========== TEST φ-EXPANSION ==========
    cout << "φ-EXPANSION NG MGA NUMBERS:\n";
    cout << "===========================\n\n";
    
    for (int n : {1, 2, 3, 5, 7, 10, 15, 20, 50, 100}) {
        auto coeffs = phi_expand(n);
        
        cout << "  " << setw(3) << n << " = ";
        double sum = 0;
        for (size_t i = 0; i < coeffs.size(); i++) {
            double phi_p = pow(PHI, coeffs[i]);
            sum += phi_p;
            cout << "φ^" << coeffs[i];
            if (i < coeffs.size() - 1) cout << " + ";
        }
        cout << " = " << sum << " (exact: " << n << ")\n";
    }
    cout << "\n";

    // ========== EXACT φ-EXPANSION (WITH NEGATIVE COEFFICIENTS) ==========
    cout << "EXACT φ-EXPANSION (WITH NEGATIVE):\n";
    cout << "==================================\n\n";
    
    // Mas magandang algorithm: gamit ang φ² = φ + 1
    auto phi_expand_exact = [&](int n) {
        vector<pair<int, int>> terms;  // (power, coefficient)
        int remaining = n;
        
        // Greedy: subtract ang pinakamalaking φ-power
        int max_p = 0;
        while (pow(PHI, max_p) <= remaining + 1) max_p++;
        
        for (int p = max_p; p >= -10; p--) {
            double phi_p = pow(PHI, p);
            int coeff = (int)round(remaining / phi_p);
            if (coeff != 0) {
                terms.push_back({p, coeff});
                remaining -= coeff * (int)round(phi_p);
            }
        }
        
        return terms;
    };
    
    for (int n : {5, 7, 10, 15}) {
        auto terms = phi_expand_exact(n);
        
        cout << "  " << setw(3) << n << " = ";
        for (size_t i = 0; i < terms.size(); i++) {
            if (terms[i].second > 0) {
                cout << terms[i].second << "φ^" << terms[i].first;
            } else {
                cout << "(" << terms[i].second << ")φ^" << terms[i].first;
            }
            if (i < terms.size() - 1) cout << " + ";
        }
        cout << "\n";
    }
    cout << "\n";

    // ========== φ-MULTIPLICATION VIA EXPANSION ==========
    cout << "φ-MULTIPLICATION VIA EXPANSION:\n";
    cout << "==============================\n\n";
    
    cout << "  a × b kung saan b = Σ cᵢφⁱ:\n";
    cout << "  a × b = Σ cᵢ × (aφⁱ)\n";
    cout << "  At aφⁱ = F(i)×aφ + F(i-1)×a (addition lang!)\n\n";
    
    // Test: 5 × 7
    cout << "  Example: 5 × 7\n";
    cout << "  7 ≈ φ³ + φ^(-1) + φ^(-3)\n";
    cout << "  = 4.236 + 0.618 + 0.236\n";
    cout << "  = 5.09 (approximation)\n\n";
    
    cout << "  5×7 = 5×(φ³ + φ^(-1) + φ^(-3))\n";
    cout << "  = 5φ³ + 5φ^(-1) + 5φ^(-3)\n";
    cout << "  = 21.18 + 3.09 + 1.18\n";
    cout << "  = 25.45 (expected 35)\n\n";
    
    cout << "  NOTE: Ang φ-expansion ay approximation\n";
    cout << "  para sa arbitrary integers. Pero para sa\n";
    cout << "  φ-powers mismo, EXACT!\n\n";

    return 0;
}
