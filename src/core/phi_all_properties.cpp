// ============================================
// φ-ALL PROPERTIES — COMPLETE EXPLORATION
// Lahat ng properties ng φ na may kaugnayan sa
// transmutation ng operasyon
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

using namespace std;

int main() {
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

    cout << "=== φ-ALL PROPERTIES ===\n";
    cout << fixed << setprecision(10);
    cout << "φ = " << PHI << "\n";
    cout << "ln(φ) = " << LN_PHI << "\n\n";

    // ============================================
    // 1. SELF-REFERENTIAL
    // φ = 1 + 1/φ
    // ============================================
    cout << "--- 1. SELF-REFERENTIAL ---\n";
    cout << "φ = " << PHI << "\n";
    cout << "1 + 1/φ = " << 1.0 + PHI_INV << "\n";
    cout << "Match: " << (abs(PHI - (1.0 + PHI_INV)) < 1e-10 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 2. SQUARE AS ADDITION
    // φ² = φ + 1
    // ============================================
    cout << "--- 2. SQUARE AS ADDITION ---\n";
    cout << "φ² = " << PHI * PHI << "\n";
    cout << "φ + 1 = " << PHI + 1.0 << "\n";
    cout << "Match: " << (abs(PHI * PHI - (PHI + 1.0)) < 1e-10 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 3. INVERSE AS SUBTRACTION
    // 1/φ = φ - 1
    // ============================================
    cout << "--- 3. INVERSE AS SUBTRACTION ---\n";
    cout << "1/φ = " << PHI_INV << "\n";
    cout << "φ - 1 = " << PHI - 1.0 << "\n";
    cout << "Match: " << (abs(PHI_INV - (PHI - 1.0)) < 1e-10 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 4. CUBE AS LINEAR
    // φ³ = 2φ + 1
    // ============================================
    cout << "--- 4. CUBE AS LINEAR ---\n";
    cout << "φ³ = " << pow(PHI, 3) << "\n";
    cout << "2φ + 1 = " << 2.0 * PHI + 1.0 << "\n";
    cout << "Match: " << (abs(pow(PHI, 3) - (2.0 * PHI + 1.0)) < 1e-10 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 5. FIBONACCI REPRESENTATION
    // φ^n = F_n × φ + F_{n-1}
    // ============================================
    cout << "--- 5. FIBONACCI REPRESENTATION ---\n";
    cout << "  n | φ^n | F_n×φ + F_{n-1} | Match\n";
    cout << "  --|-----|------------------|-------\n";
    
    long long F_prev = 0, F_curr = 1;
    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double fib_rep = (n == 0) ? 1.0 : F_curr * PHI + F_prev;
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << phi_n << " | "
             << setw(12) << fib_rep << " | "
             << (abs(phi_n - fib_rep) < 1e-8 ? "✅" : "❌") << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 6. RATIO CONVERGENCE
    // F_{n+1}/F_n → φ
    // ============================================
    cout << "--- 6. RATIO CONVERGENCE ---\n";
    cout << "  n | F_{n+1}/F_n | Error\n";
    cout << "  --|-------------|-------\n";
    
    F_prev = 1;
    F_curr = 1;
    for (int n = 1; n <= 15; n++) {
        double ratio = (double)(F_curr + F_prev) / F_curr;
        double error = abs(ratio - PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << ratio << " | "
             << setw(12) << error << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 7. BINET FORMULA
    // F_n = (φ^n - (-φ)^(-n))/√5
    // ============================================
    cout << "--- 7. BINET FORMULA ---\n";
    cout << "  n | F_n (recurrence) | F_n (Binet) | Match\n";
    cout << "  --|------------------|-------------|-------\n";
    
    F_prev = 0;
    F_curr = 1;
    for (int n = 0; n <= 15; n++) {
        double fib_recurrence = (n == 0) ? 0.0 : (n == 1) ? 1.0 : F_curr;
        double binet = (pow(PHI, n) - pow(-PHI_INV, n)) / SQRT5;
        
        cout << "  " << setw(2) << n << " | "
             << setw(18) << fib_recurrence << " | "
             << setw(12) << binet << " | "
             << (abs(fib_recurrence - binet) < 1e-8 ? "✅" : "❌") << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 8. FIBONACCI MATRIX
    // [[1,1],[1,0]]^n = [[F_{n+1}, F_n], [F_n, F_{n-1}]]
    // ============================================
    cout << "--- 8. FIBONACCI MATRIX ---\n";
    cout << "  n | F_{n+1} | F_n | F_{n-1} | Det\n";
    cout << "  --|---------|-----|---------|-----\n";
    
    F_prev = 1;
    F_curr = 1;
    long long F_prev2 = 0;
    for (int n = 1; n <= 10; n++) {
        long long det = F_curr * F_prev2 - F_prev * F_prev;
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << F_curr << " | "
             << setw(4) << F_prev << " | "
             << setw(8) << F_prev2 << " | "
             << setw(5) << det << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev2 = F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "  (Det = (-1)^n)\n\n";

    // ============================================
    // 9. CONTINUED FRACTION
    // φ = [1; 1, 1, 1, ...]
    // ============================================
    cout << "--- 9. CONTINUED FRACTION ---\n";
    cout << "  Iterations | Approximation | Error\n";
    cout << "  -----------|---------------|-------\n";
    
    double cf = 1.0;
    for (int n = 0; n <= 15; n++) {
        double error = abs(cf - PHI);
        
        cout << "  " << setw(10) << n << " | "
             << setw(14) << cf << " | "
             << setw(12) << error << "\n";
        
        cf = 1.0 + 1.0 / cf;
    }
    cout << "\n";

    // ============================================
    // 10. CONVERGENTS
    // 1/1, 2/1, 3/2, 5/3, 8/5, ...
    // ============================================
    cout << "--- 10. CONVERGENTS ---\n";
    cout << "  n | F_{n+1}/F_n | Error\n";
    cout << "  --|-------------|-------\n";
    
    F_prev = 1;
    F_curr = 1;
    for (int n = 1; n <= 10; n++) {
        double convergent = (double)(F_curr + F_prev) / F_curr;
        double error = abs(convergent - PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << convergent << " | "
             << setw(12) << error << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 11. BEATTY SEQUENCES
    // floor(n×φ) at floor(n×φ²)
    // ============================================
    cout << "--- 11. BEATTY SEQUENCES ---\n";
    cout << "  n | floor(n×φ) | floor(n×φ²) | Gap\n";
    cout << "  --|-------------|--------------|-----\n";
    
    for (int n = 1; n <= 20; n++) {
        double b1 = floor(n * PHI);
        double b2 = floor(n * PHI * PHI);
        double gap = b2 - b1;
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << b1 << " | "
             << setw(12) << b2 << " | "
             << setw(4) << gap << "\n";
    }
    cout << "\n";

    // ============================================
    // 12. COMPLEMENTARY
    // 1/φ + 1/φ² = 1
    // ============================================
    cout << "--- 12. COMPLEMENTARY ---\n";
    cout << "1/φ = " << PHI_INV << "\n";
    cout << "1/φ² = " << PHI_INV * PHI_INV << "\n";
    cout << "1/φ + 1/φ² = " << PHI_INV + PHI_INV * PHI_INV << "\n";
    cout << "Match: " << (abs(PHI_INV + PHI_INV * PHI_INV - 1.0) < 1e-10 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 13. HYPERBOLIC
    // ln(φ) = asinh(1/2)
    // ============================================
    cout << "--- 13. HYPERBOLIC ---\n";
    cout << "ln(φ) = " << LN_PHI << "\n";
    cout << "asinh(1/2) = " << asinh(0.5) << "\n";
    cout << "Match: " << (abs(LN_PHI - asinh(0.5)) < 1e-10 ? "✅" : "❌") << "\n";
    cout << "sinh(ln φ) = " << sinh(LN_PHI) << " (dapat 1/2)\n";
    cout << "cosh(ln φ) = " << cosh(LN_PHI) << " (dapat √5/2)\n\n";

    // ============================================
    // 14. HYPERBOLIC ADDITION
    // e^(a×ln φ) × e^(b×ln φ) = e^((a+b)×ln φ)
    // ============================================
    cout << "--- 14. HYPERBOLIC ADDITION ---\n";
    cout << "  a | b | φ^a × φ^b | φ^(a+b) | Match\n";
    cout << "  --|---|------------|---------|-------\n";
    
    for (int a = 1; a <= 5; a++) {
        for (int b = 1; b <= 3; b++) {
            double mult = pow(PHI, a) * pow(PHI, b);
            double add = pow(PHI, a + b);
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << setw(12) << mult << " | "
                 << setw(12) << add << " | "
                 << (abs(mult - add) < 1e-8 ? "✅" : "❌") << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 15. GOLDEN ANGLE
    // 2π/φ² ≈ 137.5°
    // ============================================
    cout << "--- 15. GOLDEN ANGLE ---\n";
    double golden_angle = 2.0 * M_PI / (PHI * PHI);
    cout << "2π/φ² = " << golden_angle << " radians\n";
    cout << "        = " << golden_angle * 180.0 / M_PI << " degrees\n";
    cout << "φ² = " << PHI * PHI << "\n";
    cout << "2π/φ² × φ² = " << golden_angle * PHI * PHI << " (dapat 2π)\n\n";

    // ============================================
    // 16. φ-NUMBER SYSTEM
    // x = Σ c_i × φ^i
    // ============================================
    cout << "--- 16. φ-NUMBER SYSTEM ---\n";
    cout << "  φ^0 = 1\n";
    cout << "  φ^1 = " << PHI << "\n";
    cout << "  φ^2 = " << PHI * PHI << "\n";
    cout << "  φ^3 = " << pow(PHI, 3) << "\n";
    cout << "  φ^(-1) = " << PHI_INV << "\n";
    cout << "  φ^(-2) = " << PHI_INV * PHI_INV << "\n\n";
    
    cout << "  Integer | φ-representation\n";
    cout << "  -------|------------------\n";
    for (int n = 1; n <= 10; n++) {
        cout << "  " << setw(6) << n << " | ";
        
        // Subukan i-represent bilang sum ng φ-powers
        double remaining = n;
        vector<int> powers;
        for (int p = 3; p >= -3; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                powers.push_back(p);
                remaining -= phi_p;
            }
        }
        
        for (size_t i = 0; i < powers.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "φ^" << powers[i];
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 17. MATRIX EIGENVALUES
    // λ₁ = φ, λ₂ = -1/φ
    // ============================================
    cout << "--- 17. MATRIX EIGENVALUES ---\n";
    cout << "λ₁ = φ = " << PHI << "\n";
    cout << "λ₂ = -1/φ = " << -PHI_INV << "\n";
    cout << "λ₁ × λ₂ = " << PHI * (-PHI_INV) << " (dapat -1)\n";
    cout << "λ₁ + λ₂ = " << PHI + (-PHI_INV) << " (dapat 1)\n\n";

    // ============================================
    // 18. MODULAR PERIODICITY
    // φ^even mod φ → 1, φ^odd mod φ → φ⁻¹
    // ============================================
    cout << "--- 18. MODULAR PERIODICITY ---\n";
    cout << "  n | φ^n mod φ | Parity\n";
    cout << "  --|-----------|--------\n";
    
    for (int n = 1; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        string parity = (n % 2 == 0) ? "even" : "odd";
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << mod_phi << " | "
             << parity << "\n";
    }
    cout << "\n";

    // ============================================
    // 19. SERIES
    // Σ φ^(-n) = φ
    // ============================================
    cout << "--- 19. SERIES ---\n";
    cout << "  n | Sum φ^(-k) | Error\n";
    cout << "  --|------------|-------\n";
    
    double sum = 0.0;
    for (int n = 0; n <= 20; n++) {
        sum += pow(PHI, -n);
        double error = abs(sum - PHI);
        
        if (n % 2 == 0 || n <= 5) {
            cout << "  " << setw(2) << n << " | "
                 << setw(12) << sum << " | "
                 << setw(12) << error << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 20. CORRECTION TERM
    // log_φ(1 + φ^(-d))
    // ============================================
    cout << "--- 20. CORRECTION TERM ---\n";
    cout << "  d | log_φ(1+φ^(-d)) | ×φ^d | Limit\n";
    cout << "  --|-----------------|-------|-------\n";
    
    double C_limit = 1.0 / LN_PHI;
    for (int d = 0; d <= 20; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double scaled = corr * pow(PHI, d);
        
        cout << "  " << setw(2) << d << " | "
             << setw(16) << corr << " | "
             << setw(10) << scaled << " | "
             << setw(10) << C_limit << "\n";
    }
    cout << "\n";

    // ============================================
    // 21. PASCAL/FIBONACCI
    // ============================================
    cout << "--- 21. PASCAL/FIBONACCI ---\n";
    cout << "  Fibonacci sa Pascal diagonals:\n";
    cout << "  Diagonal sums:\n";
    
    for (int d = 0; d <= 10; d++) {
        // Pascal diagonal sum
        long long sum_diag = 0;
        for (int k = 0; k <= d/2; k++) {
            // C(d-k, k)
            long long comb = 1;
            for (int i = 0; i < k; i++) {
                comb = comb * (d - k - i) / (i + 1);
            }
            sum_diag += comb;
        }
        
        cout << "  " << setw(2) << d << " | "
             << setw(8) << sum_diag << "\n";
    }
    cout << "  (Ito ay Fibonacci numbers: 1,1,2,3,5,8,13,21,34,55,89)\n\n";

    // ============================================
    // 22. FIXED POINT
    // x → 1 + 1/x
    // ============================================
    cout << "--- 22. FIXED POINT ---\n";
    cout << "  Iteration | x | Error\n";
    cout << "  ----------|-----|-------\n";
    
    double x = 1.0;
    for (int n = 0; n <= 15; n++) {
        double error = abs(x - PHI);
        
        cout << "  " << setw(10) << n << " | "
             << setw(12) << x << " | "
             << setw(12) << error << "\n";
        
        x = 1.0 + 1.0 / x;
    }
    cout << "\n";

    // ============================================
    // SYNTHESIS: TRANSMUTATION TABLE
    // ============================================
    cout << "=== SYNTHESIS: TRANSMUTATION TABLE ===\n\n";
    cout << "  Operation | Normal Space | Log Space | φ-Bridge\n";
    cout << "  ----------|--------------|-----------|----------\n";
    cout << "  Identity  | 1           | 0         | φ^0\n";
    cout << "  Inverse   | 1/φ         | -1        | φ⁻¹\n";
    cout << "  Square    | φ²          | 2         | φ+1\n";
    cout << "  Cube      | φ³          | 3         | 2φ+1\n";
    cout << "  Add unit  | φ+1         | log_φ(φ+1)| φ²\n";
    cout << "  Sub unit  | φ-1         | log_φ(φ-1)| φ⁻¹\n\n";

    cout << "  KEY TRANSMUTATIONS:\n";
    cout << "  φ + 1 = φ² (addition → square)\n";
    cout << "  φ - 1 = φ⁻¹ (subtraction → inverse)\n";
    cout << "  φ × φ⁻¹ = 1 (multiplication → identity)\n";
    cout << "  1 + φ⁻¹ = φ (addition → self)\n\n";

    return 0;
}
