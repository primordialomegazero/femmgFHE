// ============================================
// φ-MAX SCALED
// max(a,b) ≈ floor(max(a×φ^k, b×φ^k)) / φ^k
// para sa malaking k
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    cout << "=== φ-MAX SCALED ===\n\n";
    
    for (int k : {2, 3, 5, 8, 13, 21}) {
        double phi_k = pow(PHI, k);
        
        cout << "  k=" << setw(2) << k << " | ";
        
        int correct = 0;
        int total = 0;
        
        for (int a = 1; a <= 5; a++) {
            for (int b = 1; b <= 5; b++) {
                int m = max(a, b);
                double scaled_a = a * phi_k;
                double scaled_b = b * phi_k;
                double scaled_max = max(scaled_a, scaled_b);
                double approx = floor(scaled_max) / phi_k;
                double diff = abs(m - approx);
                
                if (diff < 0.5) correct++;
                total++;
            }
        }
        
        cout << "Correct: " << correct << "/" << total << " (";
        cout << fixed << setprecision(0) << (correct * 100.0 / total) << "%)\n";
    }
    
    cout << "\n=== PINAKAMALAKING K ===\n\n";
    {
        int k = 100;
        double phi_k = pow(PHI, k);
        
        cout << "  k=100, φ^k = " << scientific << phi_k << "\n\n";
        
        for (int a : {1, 2, 3, 5, 8, 13}) {
            for (int b : {2, 3, 5, 8}) {
                int m = max(a, b);
                double scaled_max = max(a * phi_k, b * phi_k);
                double approx = floor(scaled_max) / phi_k;
                double diff = abs(m - approx);
                
                cout << "  max(" << setw(2) << a << "," << setw(2) << b << ") = "
                     << m << ", approx = " << fixed << setprecision(10) << approx
                     << ", diff = " << diff << "\n";
            }
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang scaled max ay eksakto,\n";
    cout << "  hindi na natin kailangan ng comparison\n\n";

    return 0;
}
