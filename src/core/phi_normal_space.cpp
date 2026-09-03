// ============================================
// φ-NORMAL SPACE
// Beatty pair bilang natural na normal space
// (floor(nφ), n) — normal at log magkasama
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-NORMAL SPACE ===\n\n";
    
    // Beatty pairs para sa n=1 hanggang 20
    cout << "  n | floor(nφ) | floor(nφ²) | log_φ(floor(nφ)) | log_φ(n)+1 | Diff\n";
    cout << "  --|-----------|------------|-------------------|------------|------\n";
    
    for (int n = 1; n <= 20; n++) {
        double F_phi = floor(n * PHI);
        double F_phi2 = floor(n * PHI * PHI);
        double log_F = log(F_phi) / LN_PHI;
        double log_n_plus_1 = log(n) / LN_PHI + 1.0;
        double diff = log_F - log_n_plus_1;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(0) << F_phi << " | "
             << setw(10) << F_phi2 << " | "
             << setw(15) << log_F << " | "
             << setw(12) << log_n_plus_1 << " | "
             << setw(6) << diff << "\n";
    }
    
    cout << "\n=== ADDITION SA BEATTY SPACE ===\n\n";
    cout << "  floor(aφ) + floor(bφ) = ?\n\n";
    
    for (int a = 1; a <= 5; a++) {
        for (int b = a; b <= a+2; b++) {
            double sum = floor(a * PHI) + floor(b * PHI);
            int n_sum = (int)floor(sum / PHI);
            double log_sum = log(sum) / LN_PHI;
            double log_n_plus_1 = log(n_sum) / LN_PHI + 1.0;
            
            cout << "  " << a << "+" << b << ": floor(aφ)+floor(bφ) = " 
                 << sum << ", n=" << n_sum 
                 << ", log=" << fixed << setprecision(3) << log_sum
                 << ", approx=" << log_n_plus_1
                 << ", diff=" << (log_sum - log_n_plus_1) << "\n";
        }
    }
    
    cout << "\n=== MULTIPLICATION SA BEATTY SPACE ===\n\n";
    cout << "  floor(aφ) × floor(bφ) = ?\n\n";
    
    for (int a = 1; a <= 3; a++) {
        for (int b = a; b <= a+1; b++) {
            double prod = floor(a * PHI) * floor(b * PHI);
            int n_prod = (int)floor(prod / PHI);
            double log_prod = log(prod) / LN_PHI;
            double log_a_plus_1 = log(a) / LN_PHI + 1.0;
            double log_b_plus_1 = log(b) / LN_PHI + 1.0;
            double log_sum = log_a_plus_1 + log_b_plus_1;
            
            cout << "  " << a << "×" << b << ": floor(aφ)×floor(bφ) = " 
                 << prod << ", n=" << n_prod 
                 << ", log=" << fixed << setprecision(3) << log_prod
                 << ", approx=" << log_sum
                 << ", diff=" << (log_prod - log_sum) << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay 0, ang Beatty pair\n";
    cout << "  ay nagbibigay ng natural na normal at log space\n\n";

    return 0;
}
