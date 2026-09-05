// ============================================
// φ-INTEGER ADDITION
// Hanapin ang constant correction para sa
// integer addition sa φ-space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-INTEGER ADDITION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Lucas numbers: φ^n + φ^(-n)
    // ============================================
    cout << "--- 1. Lucas numbers ---\n\n";
    cout << "  n | φ^n + φ^(-n) | Integer?\n";
    cout << "  --|---------------|----------\n";
    
    for (int n = 0; n <= 10; n++) {
        double lucas = pow(PHI, n) + pow(PHI, -n);
        bool is_int = abs(lucas - round(lucas)) < 1e-10;
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << lucas << " | "
             << (is_int ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. φ^n - φ^(-n)
    // ============================================
    cout << "--- 2. φ^n - φ^(-n) ---\n\n";
    cout << "  n | φ^n - φ^(-n) | ×√5?\n";
    cout << "  --|---------------|-------\n";
    
    for (int n = 0; n <= 10; n++) {
        double diff = pow(PHI, n) - pow(PHI, -n);
        double times_sqrt5 = diff * sqrt(5.0);
        bool is_fib = abs(times_sqrt5 - round(times_sqrt5)) < 1e-10;
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << diff << " | "
             << setw(14) << times_sqrt5 << " | "
             << (is_fib ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. φ^n + φ^(n-2)
    // ============================================
    cout << "--- 3. φ^n + φ^(n-2) ---\n\n";
    cout << "  n | φ^n + φ^(n-2) | φ^(n-2) × ?\n";
    cout << "  --|----------------|------------\n";
    
    for (int n = 2; n <= 12; n++) {
        double sum = pow(PHI, n) + pow(PHI, n-2);
        double factor = sum / pow(PHI, n-2);
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << sum << " | "
             << setw(14) << factor << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Integer decomposition sa φ-powers
    // ============================================
    cout << "--- 4. Integer decomposition ---\n\n";
    cout << "  Integer | φ-power decomposition\n";
    cout << "  -------|----------------------\n";
    
    for (int x = 1; x <= 15; x++) {
        cout << "  " << setw(6) << x << " | ";
        
        double remaining = x;
        vector<int> powers;
        
        // Hanapin ang φ-power decomposition
        for (int p = 5; p >= -5; p--) {
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
    // 5. Ang correction para sa φ^n + integer
    // ============================================
    cout << "--- 5. Correction para sa φ^n + integer ---\n\n";
    cout << "  n | x | log_φ(φ^n + x) | n + ?\n";
    cout << "  --|---|------------------|------\n";
    
    for (int n = 1; n <= 5; n++) {
        for (int x : {1, 2, 3, 5}) {
            double phi_n = pow(PHI, n);
            double sum = phi_n + x;
            double log_sum = log(sum) / LN_PHI;
            double correction = log_sum - n;
            
            cout << "  " << setw(1) << n << " | "
                 << setw(1) << x << " | "
                 << setw(16) << log_sum << " | "
                 << "+" << setw(10) << correction << "\n";
        }
    }
    cout << "\n";

    return 0;
}
