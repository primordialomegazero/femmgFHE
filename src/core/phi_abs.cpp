// ============================================
// φ-ABS
// Hanapin ang φ-based na absolute difference
// |φ^a - φ^b| = φ^max(a,b) × |1 - φ^(-d)|
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-ABS ===\n\n";
    
    cout << "  |φ^a - φ^b| = φ^max(a,b) × |1 - φ^(-d)|\n";
    cout << "  kung saan d = |a-b|\n\n";
    
    cout << "  d | |1 - φ^(-d)| | log_φ(|1 - φ^(-d)|)\n";
    cout << "  --|-------------|--------------------\n";
    
    vector<double> abs_table(20, 0.0);
    vector<double> log_abs_table(20, 0.0);
    
    for (int d = 0; d <= 10; d++) {
        abs_table[d] = abs(1.0 - pow(PHI, -d));
        log_abs_table[d] = log(abs_table[d]) / LN_PHI;
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(4) << abs_table[d] << " | "
             << setw(12) << log_abs_table[d] << "\n";
    }
    
    cout << "\n=== TEST: |φ^a - φ^b| ===\n\n";
    cout << "  a | b | d | exact | φ^max × table[d] | Diff\n";
    cout << "  --|---|----|-------|------------------|------\n";
    
    for (int a : {1, 2, 3, 5, 8}) {
        for (int b : {a, a+1, a+2}) {
            int d = abs(a - b);
            double exact = abs(pow(PHI, a) - pow(PHI, b));
            double approx = pow(PHI, max(a,b)) * abs_table[d];
            double diff = exact - approx;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(2) << d << " | "
                 << setw(6) << fixed << setprecision(2) << exact << " | "
                 << setw(8) << approx << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang |φ^a - φ^b| ay may eksaktong formula\n";
    cout << "  na nakadepende lang sa d = |a-b|\n\n";

    return 0;
}
