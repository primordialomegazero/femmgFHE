// ============================================
// φ-EXACT ADD
// Hanapin ang EXACT na formula para sa
// addition sa log space
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
    cout << "  φ-EXACT ADD\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double INV_SQRT5 = 1.0 / SQRT5;

    // ============================================
    // TEST 1: EXACT FORMULA VIA BINET
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT FORMULA VIA BINET\n";
    cout << "========================================\n\n";

    cout << "  Binet: F_n = (φ^n - ψ^n) / √5, ψ = -1/φ\n";
    cout << "  Lucas: L_n = φ^n + ψ^n\n\n";

    cout << "  Para sa addition a+b:\n";
    cout << "  a+b = F_m + F_n = ?\n";
    cout << "  a+b = L_m + L_n = ?\n";
    cout << "  a+b = F_m + L_n = ?\n\n";

    // ============================================
    // TEST 2: LOG-SPACE ADDITION IDENTITY
    // ============================================

    cout << "========================================\n";
    cout << "  LOG-SPACE ADDITION IDENTITY\n";
    cout << "========================================\n\n";

    cout << "  log_φ(a+b) = log_φ(a(1+b/a))\n";
    cout << "  = log_φ(a) + log_φ(1+b/a)\n\n";

    cout << "  Ang key: log_φ(1+b/a)\n";
    cout << "  Kapag b/a = φ^k:\n";
    cout << "  1+φ^k = ?\n\n";

    for (double k : {-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0}) {
        double phi_k = pow(PHI, k);
        double one_plus = 1 + phi_k;
        double log_val = log(one_plus) / LN_PHI;
        
        cout << "  k=" << setw(4) << k 
             << " | φ^k=" << setw(8) << fixed << setprecision(4) << phi_k
             << " | 1+φ^k=" << setw(8) << one_plus
             << " | log_φ=" << setw(8) << log_val << "\n";
    }

    // ============================================
    // TEST 3: SELF-REFERENTIAL ADDITION
    // ============================================

    cout << "\n========================================\n";
    cout << "  SELF-REFERENTIAL ADDITION\n";
    cout << "========================================\n\n";

    cout << "  φ² = φ + 1\n";
    cout << "  Kaya: 1 + φ = φ² → log_φ(1+φ) = 2\n";
    cout << "  Kaya: 1 + 1/φ = φ → log_φ(1+1/φ) = 1\n\n";

    cout << "  Para sa arbitrary a, b:\n";
    cout << "  a + b = a(1 + b/a)\n";
    cout << "  Kung b/a ay kayang i-represent bilang φ^k:\n";
    cout << "  log_φ(a+b) = log_φ(a) + log_φ(1+φ^k)\n\n";

    // ============================================
    // TEST 4: FIBONACCI ADDITION FORMULA
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI ADDITION FORMULA\n";
    cout << "========================================\n\n";

    cout << "  F_m + F_n = ?\n";
    cout << "  m | n | F_m | F_n | F_m+F_n | Nearest F/L\n";
    cout << "  --|---|-----|-----|---------|----------\n";

    vector<long long> fib = {0, 1};
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    for (int m = 2; m <= 8; m++) {
        for (int n = m; n <= 8; n++) {
            long long sum = fib[m] + fib[n];
            
            bool is_fib = false;
            bool is_lucas = false;
            int idx_f = 0, idx_l = 0;
            
            for (int i = 0; i < fib.size(); i++) {
                if (fib[i] == sum) { is_fib = true; idx_f = i; break; }
            }
            for (int i = 0; i < lucas.size(); i++) {
                if (lucas[i] == sum) { is_lucas = true; idx_l = i; break; }
            }
            
            cout << "  " << setw(2) << m << " | " << setw(1) << n
                 << " | " << setw(3) << fib[m]
                 << " | " << setw(3) << fib[n]
                 << " | " << setw(6) << sum
                 << " | " << (is_fib ? "F_" + to_string(idx_f) : "")
                 << (is_lucas ? " L_" + to_string(idx_l) : "")
                 << "\n";
        }
    }

    // ============================================
    // TEST 5: EXACT DECOMPOSITION
    // ============================================

    cout << "\n========================================\n";
    cout << "  EXACT DECOMPOSITION\n";
    cout << "========================================\n\n";

    cout << "  a + b = c\n";
    cout << "  log_φ(c) = log_φ(φ^m + φ^n) para sa Lucas-like\n";
    cout << "  o log_φ(φ^m - φ^n) para sa Fibonacci-like\n\n";

    cout << "  a | b | a+b | φ-decomposition\n";
    cout << "  --|---|-----|----------------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double sum = a + b;
            
            // Hanapin ang pinakamalapit na φ^m + φ^n o φ^m - φ^n
            double best_val = 0;
            double best_m = 0, best_n = 0;
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
                        best_form = "φ^" + to_string(m) + " + φ^" + to_string(n);
                        best_val = sum_phi;
                    }
                    if (abs(diff_phi - sum) < min_err) {
                        min_err = abs(diff_phi - sum);
                        best_form = "φ^" + to_string(m) + " - φ^" + to_string(n);
                        best_val = diff_phi;
                    }
                }
            }
            
            cout << "  " << setw(2) << a << " | " << setw(1) << b
                 << " | " << setw(3) << sum
                 << " | " << best_form << " = " << fixed << setprecision(4) << best_val
                 << " (err=" << min_err << ")\n";
        }
    }

    return 0;
}
