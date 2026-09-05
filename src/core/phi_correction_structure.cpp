// ============================================
// φ-CORRECTION STRUCTURE
// Hanapin ang exact structure ng corrections
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double C = 1.0 / LN_PHI;  // 2.078...

    cout << "=== φ-CORRECTION STRUCTURE ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. corr(k) × φ^k — may limit ba?
    // ============================================
    cout << "--- 1. corr(k) × φ^k ---\n\n";
    cout << "  k | corr(k) × φ^k | Diff mula sa C\n";
    cout << "  --|----------------|-----------------\n";
    
    vector<double> corr(20, 0.0);
    for (int k = 1; k <= 19; k++) {
        corr[k] = log(1.0 + pow(PHI, -k)) / LN_PHI;
        double scaled = corr[k] * pow(PHI, k);
        double diff = scaled - C;
        
        cout << "  " << setw(2) << k << " | "
             << setw(15) << scaled << " | "
             << setw(15) << diff << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. corr(k) — φ^(1-k) — may pattern?
    // ============================================
    cout << "--- 2. corr(k) - φ^(1-k) ---\n\n";
    cout << "  k | corr(k) - φ^(1-k) | ×φ^(2k)\n";
    cout << "  --|-------------------|---------\n";
    
    for (int k = 1; k <= 15; k++) {
        double phi_pow = pow(PHI, 1-k);
        double diff = corr[k] - phi_pow;
        double scaled = diff * pow(PHI, 2*k);
        
        cout << "  " << setw(2) << k << " | "
             << setw(18) << diff << " | "
             << setw(15) << scaled << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang exact formula
    // ============================================
    cout << "--- 3. Exact formula hunt ---\n\n";
    cout << "  corr(k) = log_φ(1 + φ^(-k))\n";
    cout << "          = log_φ(φ^k + 1) - k\n";
    cout << "          = log_φ(F_{k+1} + F_k×φ) - k  (Fibonacci!)\n\n";
    
    // Check: φ^k + 1 = F_{k+1} + F_k×φ
    cout << "  k | φ^k + 1 | F_{k+1} + F_k×φ | Match\n";
    cout << "  --|---------|------------------|-------\n";
    
    long long F_prev = 1, F_curr = 1;
    for (int k = 1; k <= 10; k++) {
        double phi_k = pow(PHI, k);
        double fib_rep = F_curr * PHI + F_prev;
        
        cout << "  " << setw(2) << k << " | "
             << setw(9) << phi_k + 1 << " | "
             << setw(12) << fib_rep << " | "
             << (abs(phi_k + 1 - fib_rep) < 1e-8 ? "✅" : "❌") << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 4. corr(k) bilang Fibonacci ratio
    // ============================================
    cout << "--- 4. corr(k) bilang Fibonacci ---\n\n";
    cout << "  k | corr(k) | Fibonacci representation\n";
    cout << "  --|---------|------------------------\n";
    
    F_prev = 1; F_curr = 1;
    for (int k = 1; k <= 10; k++) {
        cout << "  " << setw(2) << k << " | "
             << setw(12) << corr[k] << " | ";
        
        // Hanapin kung corr(k) ≈ F_m/F_n
        double best_ratio = 0;
        int best_m = 0, best_n = 0;
        double best_diff = 999;
        
        for (int m = 1; m <= 20; m++) {
            for (int n = 1; n <= 20; n++) {
                long long f_m = 0, f_n = 0;
                long long a = 0, b = 1;
                for (int i = 2; i <= m; i++) { long long t = a+b; a=b; b=t; }
                f_m = b;
                a = 0; b = 1;
                for (int i = 2; i <= n; i++) { long long t = a+b; a=b; b=t; }
                f_n = b;
                
                double ratio = (double)f_m / f_n;
                double diff = abs(ratio - corr[k]);
                if (diff < best_diff) {
                    best_diff = diff;
                    best_ratio = ratio;
                    best_m = m;
                    best_n = n;
                }
            }
        }
        
        cout << "F_" << best_m << "/F_" << best_n << " = " << best_ratio;
        cout << " (diff: " << best_diff << ")\n";
    }
    cout << "\n";

    return 0;
}
