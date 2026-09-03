// ============================================
// φ-HOMOMORPHIC MOD SEARCH
// Hanapin ang homomorphic modulo φ
// at mga hindi tugmang φ-properties
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-HOMOMORPHIC MOD SEARCH ===\n\n";

    // ============================================
    // 1. φ-PERIODICITY NG OPERATIONS
    // ============================================
    cout << "--- 1. φ-PERIODICITY NG OPERATIONS ---\n\n";
    cout << "  May pattern ba kapag nag-operate sa mod φ space?\n\n";
    
    cout << "  n | φ^n mod φ | φ^n mod φ² | φ^n mod φ³\n";
    cout << "  --|-----------|------------|-----------\n";
    
    for (int n = 1; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << fmod(phi_n, PHI) << " | "
             << setw(10) << fmod(phi_n, PHI*PHI) << " | "
             << setw(10) << fmod(phi_n, PHI*PHI*PHI) << "\n";
    }

    // ============================================
    // 2. FIBONACCI MOD PHI — HINDI TUGMA
    // ============================================
    cout << "\n--- 2. FIBONACCI MOD PHI ---\n\n";
    cout << "  F_n mod φ ay may pattern, pero ano ang F_n mod φ²?\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  n | F_n | F_n mod φ | F_n mod φ²\n";
    cout << "  --|-----|-----------|------------\n";
    
    for (int n = 3; n <= 15; n++) {
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << setw(9) << fixed << setprecision(4) << fmod((double)fib[n], PHI) << " | "
             << setw(10) << fmod((double)fib[n], PHI*PHI) << "\n";
    }

    // ============================================
    // 3. ANG HINDI TUGMA: LOG SPACE VS NORMAL SPACE
    // ============================================
    cout << "\n--- 3. LOG SPACE VS NORMAL SPACE ---\n\n";
    cout << "  Kapag nag-add sa normal space, ang log space ay hindi\n";
    cout << "  nag-a-add — kaya may divergence\n\n";
    
    cout << "  F₁ | F₂ | F₁+F₂ | log(F₁+F₂) | log(F₁)+log(F₂) | Divergence\n";
    cout << "  ----|----|-------|------------|-----------------|-----------\n";
    
    for (double F1 : {2.0, 3.0, 5.0}) {
        for (double F2 : {2.0, 3.0, 5.0}) {
            double sum = F1 + F2;
            double log_sum = log(sum) / LN_PHI;
            double log_product = log(F1) / LN_PHI + log(F2) / LN_PHI;
            double divergence = log_product - log_sum;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << sum << " | "
                 << setw(10) << fixed << setprecision(3) << log_sum << " | "
                 << setw(10) << log_product << " | "
                 << setw(8) << divergence << "\n";
        }
    }

    // ============================================
    // 4. ANG HINDI TUGMA: MOD PHI + ADDITION
    // ============================================
    cout << "\n--- 4. MOD PHI + ADDITION ---\n\n";
    cout << "  (F₁ + F₂) mod φ vs (F₁ mod φ + F₂ mod φ) mod φ\n\n";
    
    cout << "  F₁ | F₂ | (F₁+F₂) mod φ | (F₁ mod φ + F₂ mod φ) mod φ | Match?\n";
    cout << "  ----|----|---------------|-------------------------------|-------\n";
    
    for (double F1 : {2.0, 3.0, 5.0, 8.0}) {
        for (double F2 : {2.0, 3.0, 5.0}) {
            double direct = fmod(F1 + F2, PHI);
            double modded = fmod(fmod(F1, PHI) + fmod(F2, PHI), PHI);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(10) << fixed << setprecision(4) << direct << " | "
                 << setw(10) << modded << " | "
                 << (abs(direct - modded) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY OBSERVATIONS ===\n";
    cout << "  Hanapin kung aling property ang may nakatagong\n";
    cout << "  homomorphic modulo behavior\n\n";

    return 0;
}
