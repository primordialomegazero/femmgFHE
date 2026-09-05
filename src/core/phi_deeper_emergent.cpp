// ============================================
// φ-DEEPER EMERGENT
// Hanapin ang mas malalim na emergent properties
// na nagbibigay ng arbitrary addition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-DEEPER EMERGENT ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Fibonacci identities na may φ
    // ============================================
    cout << "--- 1. Fibonacci identities ---\n\n";
    cout << "  F_n = (φ^n - (-φ)^(-n))/√5\n";
    cout << "  φ^n = F_n × φ + F_{n-1}\n\n";
    
    cout << "  n | φ^n | F_n×φ + F_{n-1} | Match\n";
    cout << "  --|-----|------------------|-------\n";
    
    long long F_prev = 0, F_curr = 1;
    for (int n = 0; n <= 12; n++) {
        double phi_n = pow(PHI, n);
        double fib_rep = (n == 0) ? 1.0 : F_curr * PHI + F_prev;
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << phi_n << " | "
             << setw(14) << fib_rep << " | "
             << (abs(phi_n - fib_rep) < 1e-10 ? "✅" : "❌") << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 2. φ^n + φ^m — general addition
    // ============================================
    cout << "--- 2. φ^n + φ^m (general) ---\n\n";
    cout << "  φ^n + φ^m = φ^max × (1 + φ^(-|n-m|))\n";
    cout << "  Kung |n-m| = k, correction = log_φ(1 + φ^(-k))\n\n";
    
    cout << "  n | m | |n-m| | correction\n";
    cout << "  --|---|-------|-----------\n";
    
    for (int n : {3, 5, 8}) {
        for (int m : {2, 4, 7}) {
            int k = abs(n - m);
            double correction = log(1.0 + pow(PHI, -k)) / LN_PHI;
            
            cout << "  " << setw(1) << n << " | "
                 << setw(1) << m << " | "
                 << setw(5) << k << " | "
                 << setw(12) << correction << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ^n + 1 — special cases
    // ============================================
    cout << "--- 3. φ^n + 1 (special cases) ---\n\n";
    cout << "  n | φ^n + 1 | Formula\n";
    cout << "  --|---------|--------\n";
    
    for (int n = 0; n <= 10; n++) {
        double sum = pow(PHI, n) + 1.0;
        string formula;
        
        if (n == 0) formula = "2 = φ + φ⁻¹";
        else if (n == 1) formula = "φ + 1 = φ²";
        else if (n == 2) formula = "φ² + 1 = φ² + φ⁰";
        else if (n == 3) formula = "φ³ + 1 = φ² + φ⁻²";
        else if (n == 4) formula = "φ⁴ + 1 = φ⁴ + φ⁰";
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << sum << " | "
             << formula << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang φ bilang linear operator
    // ============================================
    cout << "--- 4. φ bilang linear operator ---\n\n";
    cout << "  φ(a + bφ) = aφ + bφ² = aφ + b(φ + 1) = b + (a+b)φ\n\n";
    cout << "  Ito ay Fibonacci-like transformation:\n";
    cout << "  (a, b) → (b, a+b)\n\n";
    
    cout << "  Step | (a, b) | Value\n";
    cout << "  -----|--------|-------\n";
    
    double a = 1, b = 0;
    for (int i = 0; i <= 10; i++) {
        double value = a + b * PHI;
        cout << "  " << setw(4) << i << " | ("
             << setw(3) << a << ", " << setw(3) << b << ") | "
             << setw(12) << value << "\n";
        
        double new_a = b;
        double new_b = a + b;
        a = new_a;
        b = new_b;
    }
    cout << "\n";

    // ============================================
    // 5. Ang φ-power ladder bilang universal
    // ============================================
    cout << "--- 5. φ-power ladder ---\n\n";
    cout << "  Ang φ-power ladder ay maaaring mag-encode\n";
    cout << "  ng arbitrary integers bilang sum ng φ-powers\n\n";
    
    cout << "  Integer | φ-power sum\n";
    cout << "  -------|------------\n";
    
    for (int x = 1; x <= 20; x++) {
        cout << "  " << setw(6) << x << " | ";
        
        double remaining = x;
        bool first = true;
        for (int p = 10; p >= -10; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                if (!first) cout << " + ";
                cout << "φ^" << p;
                first = false;
                remaining -= phi_p;
            }
        }
        cout << "  (err: " << remaining << ")\n";
    }
    cout << "\n";

    // ============================================
    // 6. Ang emergent na addition formula
    // ============================================
    cout << "--- 6. Emergent addition formula ---\n\n";
    cout << "  φ^n + φ^m = φ^(n+m-|n-m|) / (1 + φ^(-|n-m|))⁻¹ × ...\n";
    cout << "  Mas simple:\n";
    cout << "  φ^n + φ^m = φ^min × (1 + φ^(|n-m|))\n\n";
    
    cout << "  n=5, m=3: φ⁵ + φ³ = φ³ × (1 + φ²)\n";
    cout << "  = " << pow(PHI, 5) + pow(PHI, 3) << "\n";
    cout << "  = " << pow(PHI, 3) * (1 + pow(PHI, 2)) << "\n";
    cout << "  Match: " << (abs(pow(PHI,5)+pow(PHI,3) - pow(PHI,3)*(1+pow(PHI,2))) < 1e-10 ? "✅" : "❌") << "\n\n";

    return 0;
}
