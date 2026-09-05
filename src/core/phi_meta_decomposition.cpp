// ============================================
// φ-META DECOMPOSITION
// Mas refined na φ-gap decomposition
// Hanapin ang exact na φ-structure
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-META DECOMPOSITION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang φ-gaps na may negative powers
    // ============================================
    cout << "--- 1. φ-gaps na may negative powers ---\n\n";
    cout << "  φ^n + (φ^(n+k) - φ^n) = φ^(n+k)\n";
    cout << "  Kung k ay negative: φ^(n-k) - φ^n\n\n";
    
    cout << "  n=4, k=-1: φ³ - φ⁴ = " << pow(PHI,3) - pow(PHI,4) << "\n";
    cout << "  n=4, k=-2: φ² - φ⁴ = " << pow(PHI,2) - pow(PHI,4) << "\n";
    cout << "  n=4, k=-3: φ¹ - φ⁴ = " << pow(PHI,1) - pow(PHI,4) << "\n\n";

    // ============================================
    // 2. Ang exact φ-gap decomposition
    // ============================================
    cout << "--- 2. Exact φ-gap decomposition ---\n\n";
    cout << "  Integer | Exact decomposition | Error\n";
    cout << "  -------|--------------------|-------\n";
    
    auto exact_decompose = [&](int x) {
        vector<pair<int,int>> gaps;
        double remaining = x;
        
        // Subukan LAHAT ng possible na φ-gaps
        for (int iter = 0; iter < 50 && abs(remaining) > 1e-10; iter++) {
            bool found = false;
            
            // Hanapin ang EXACT na gap
            for (int n = -20; n <= 20; n++) {
                for (int k = -20; k <= 20; k++) {
                    if (k == 0) continue;
                    double gap = pow(PHI, n+k) - pow(PHI, n);
                    
                    if (abs(gap - remaining) < 1e-10) {
                        gaps.push_back({n, k});
                        remaining = 0;
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            
            if (!found) {
                // Hanapin ang pinakamalapit na gap
                double best_gap = 0;
                int best_n = 0, best_k = 0;
                double best_diff = 999;
                
                for (int n = -20; n <= 20; n++) {
                    for (int k = -20; k <= 20; k++) {
                        if (k == 0) continue;
                        double gap = pow(PHI, n+k) - pow(PHI, n);
                        if (gap <= remaining + 1e-6 && abs(gap - remaining) < best_diff) {
                            best_diff = abs(gap - remaining);
                            best_gap = gap;
                            best_n = n;
                            best_k = k;
                        }
                    }
                }
                
                if (best_gap != 0) {
                    gaps.push_back({best_n, best_k});
                    remaining -= best_gap;
                }
            }
        }
        
        return make_pair(gaps, remaining);
    };
    
    for (int x : {1, 2, 3, 5, 8, 13, 21, 34, 55}) {
        auto [gaps, err] = exact_decompose(x);
        
        cout << "  " << setw(3) << x << " | ";
        for (size_t i = 0; i < gaps.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "(φ^" << gaps[i].first + gaps[i].second 
                 << "-φ^" << gaps[i].first << ")";
        }
        cout << " | " << err << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang Fibonacci na exact na gaps
    // ============================================
    cout << "--- 3. Fibonacci na exact gaps ---\n\n";
    cout << "  Fibonacci numbers ay may exact na decomposition\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  F_n | Exact gap decomposition\n";
    cout << "  ----|------------------------\n";
    
    for (int i = 2; i <= 10; i++) {
        int x = fib[i];
        auto [gaps, err] = exact_decompose(x);
        
        cout << "  F_" << i << "=" << setw(3) << x << " | ";
        for (size_t j = 0; j < gaps.size(); j++) {
            if (j > 0) cout << " + ";
            cout << "(φ^" << gaps[j].first + gaps[j].second 
                 << "-φ^" << gaps[j].first << ")";
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang meta pattern
    // ============================================
    cout << "--- 4. Meta pattern ---\n\n";
    cout << "  Ang φ-gaps ay may recursive na structure:\n";
    cout << "  φ^(n+1) - φ^n = φ^(n-1)\n";
    cout << "  φ^(n+2) - φ^n = φ^(n-1) + φ^n = φ^(n+1)\n";
    cout << "  φ^(n+3) - φ^n = φ^(n+2) - φ^(n-2)\n\n";
    
    cout << "  k | φ^(n+k) - φ^n | Simplified\n";
    cout << "  --|----------------|----------\n";
    
    for (int k = 1; k <= 8; k++) {
        double gap = pow(PHI, 4+k) - pow(PHI, 4);
        double simplified = pow(PHI, 3) * (pow(PHI, k) - pow(PHI, 1));
        
        cout << "  " << k << " | "
             << setw(14) << gap << " | "
             << setw(14) << simplified << "\n";
    }
    cout << "\n";

    return 0;
}
