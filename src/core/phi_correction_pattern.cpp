// ============================================
// φ-CORRECTION PATTERN
// Hanapin ang pattern ng correction
// sa φ-decomposition ng addition
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
    cout << "  φ-CORRECTION PATTERN\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  a | b | a+b | φ-decomp | correction | correction/φ\n";
    cout << "  --|---|-----|----------|-----------|------------\n";

    vector<pair<double, double>> pairs = {
        {2,3}, {2,5}, {3,5}, {3,8}, {5,8}, {8,13}, {5,13}, {2,8}
    };

    for (auto [a, b] : pairs) {
        double sum = a + b;
        
        // Hanapin ang pinakamalapit na φ^m + φ^n
        double best_val = 0;
        double min_err = 1e9;
        string best_form = "";
        
        for (int m = 0; m <= 10; m++) {
            for (int n = 0; n <= 10; n++) {
                double phi_m = pow(PHI, m);
                double phi_n = pow(PHI, n);
                
                double sum_phi = phi_m + phi_n;
                double diff_phi = phi_m - phi_n;
                
                if (abs(sum_phi - sum) < min_err) {
                    min_err = abs(sum_phi - sum);
                    best_form = "φ^" + to_string(m) + "+φ^" + to_string(n);
                    best_val = sum_phi;
                }
                if (abs(diff_phi - sum) < min_err) {
                    min_err = abs(diff_phi - sum);
                    best_form = "φ^" + to_string(m) + "-φ^" + to_string(n);
                    best_val = diff_phi;
                }
            }
        }
        
        double correction = sum - best_val;
        double corr_div_phi = correction / PHI;
        
        cout << "  " << setw(2) << a << " | " << setw(2) << b
             << " | " << setw(3) << sum
             << " | " << best_form
             << " | " << fixed << setprecision(4) << correction
             << " | " << corr_div_phi << "\n";
    }

    // ============================================
    // CORRECTION = FIBONACCI FRACTION?
    // ============================================

    cout << "\n========================================\n";
    cout << "  CORRECTION = FIBONACCI FRACTION?\n";
    cout << "========================================\n\n";

    cout << "  correction × φ = Fibonacci-related?\n";
    cout << "  a | b | correction | correction×φ | nearest F\n";
    cout << "  --|---|-----------|-------------|----------\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    for (auto [a, b] : pairs) {
        double sum = a + b;
        
        double best_val = 0;
        double min_err = 1e9;
        
        for (int m = 0; m <= 10; m++) {
            for (int n = 0; n <= 10; n++) {
                double val = pow(PHI, m) + pow(PHI, n);
                if (abs(val - sum) < min_err) {
                    min_err = abs(val - sum);
                    best_val = val;
                }
            }
        }
        
        double correction = sum - best_val;
        double corr_times_phi = correction * PHI;
        
        long long nearest_f = 0;
        double min_f = 1e9;
        for (long long f : fib) {
            double d = abs(f - abs(corr_times_phi));
            if (d < min_f) { min_f = d; nearest_f = f; }
        }
        
        cout << "  " << setw(2) << a << " | " << setw(2) << b
             << " | " << fixed << setprecision(4) << correction
             << " | " << fixed << setprecision(4) << corr_times_phi
             << " | F=" << nearest_f << " (err=" << min_f << ")\n";
    }

    return 0;
}
