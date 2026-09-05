// ============================================
// φ-GAP DECOMPOSITION
// Hanapin kung lahat ng integers ay
// φ-gaps na may constant correction
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-GAP DECOMPOSITION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang φ-gaps: φ^(n+k) - φ^n
    // ============================================
    cout << "--- 1. φ-gaps ---\n\n";
    cout << "  φ^(n+k) - φ^n = φ^n × (φ^k - 1)\n\n";
    
    cout << "  n=4, k=1: φ⁵ - φ⁴ = " << pow(PHI,5) - pow(PHI,4) << "\n";
    cout << "  n=4, k=2: φ⁶ - φ⁴ = " << pow(PHI,6) - pow(PHI,4) << "\n";
    cout << "  n=4, k=3: φ⁷ - φ⁴ = " << pow(PHI,7) - pow(PHI,4) << "\n";
    cout << "  n=4, k=4: φ⁸ - φ⁴ = " << pow(PHI,8) - pow(PHI,4) << "\n\n";

    // ============================================
    // 2. Integer decomposition bilang sum ng φ-gaps
    // ============================================
    cout << "--- 2. Integer decomposition bilang φ-gaps ---\n\n";
    cout << "  Integer | φ-gap decomposition\n";
    cout << "  -------|-------------------\n";
    
    for (int x = 1; x <= 30; x++) {
        cout << "  " << setw(6) << x << " | ";
        
        double remaining = x;
        vector<pair<int,int>> gaps;
        
        // Hanapin ang pinakamalapit na φ-gap
        for (int iter = 0; iter < 10 && remaining > 1e-6; iter++) {
            bool found = false;
            for (int k = 1; k <= 10; k++) {
                for (int n = -5; n <= 10; n++) {
                    double gap = pow(PHI, n+k) - pow(PHI, n);
                    if (abs(gap - remaining) < 0.01) {
                        gaps.push_back({n, k});
                        remaining -= gap;
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
                
                for (int k = 1; k <= 10; k++) {
                    for (int n = -5; n <= 10; n++) {
                        double gap = pow(PHI, n+k) - pow(PHI, n);
                        if (gap <= remaining + 0.01 && abs(gap - remaining) < best_diff) {
                            best_diff = abs(gap - remaining);
                            best_gap = gap;
                            best_n = n;
                            best_k = k;
                        }
                    }
                }
                
                if (best_gap > 0) {
                    gaps.push_back({best_n, best_k});
                    remaining -= best_gap;
                }
            }
        }
        
        for (size_t i = 0; i < gaps.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "(φ^" << gaps[i].first + gaps[i].second 
                 << "-φ^" << gaps[i].first << ")";
        }
        cout << "  [err: " << remaining << "]\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang correction para sa φ-gaps
    // ============================================
    cout << "--- 3. Correction para sa φ-gaps ---\n\n";
    cout << "  φ^n + gap(n,k) = φ^n + φ^(n+k) - φ^n = φ^(n+k)\n";
    cout << "  Correction = k (constant!)\n\n";
    
    cout << "  gap | correction\n";
    cout << "  ----|-----------\n";
    
    for (int k = 1; k <= 5; k++) {
        cout << "  φ^(n+" << k << ") - φ^n | " << k << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang emergent na algorithm
    // ============================================
    cout << "--- 4. Emergent algorithm ---\n\n";
    cout << "  1. I-decompose ang x sa φ-gaps\n";
    cout << "  2. Bawat gap ay may constant correction k\n";
    cout << "  3. I-apply ang corrections sa exponent\n";
    cout << "  4. Lahat ay EvalAdd ng constants\n\n";

    return 0;
}
