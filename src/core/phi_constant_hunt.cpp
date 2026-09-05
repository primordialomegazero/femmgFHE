// ============================================
// φ-CONSTANT HUNT
// Hanapin ang eksaktong kahulugan ng 2.078
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CONSTANT HUNT ===\n\n";
    
    // Ang corr(d) × φ^d → C ≈ 2.078
    // Hanapin ang φ-based na kahulugan ng C
    
    double C = 2.078;
    
    cout << "  C ≈ " << C << "\n\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  φ² = " << PHI * PHI << "\n";
    cout << "  φ³ = " << pow(PHI, 3) << "\n\n";
    
    cout << "  C / φ = " << C / PHI << "\n";
    cout << "  C / φ² = " << C / (PHI * PHI) << "\n";
    cout << "  C - φ = " << C - PHI << "\n";
    cout << "  C - 1 = " << C - 1.0 << "\n\n";
    
    cout << "  √5 = " << sqrt(5.0) << "\n";
    cout << "  C × √5 = " << C * sqrt(5.0) << "\n";
    cout << "  C / √5 = " << C / sqrt(5.0) << "\n\n";
    
    cout << "  log_φ(C) = " << log(C) / LN_PHI << "\n";
    cout << "  C = φ^? = " << pow(PHI, log(C) / LN_PHI) << "\n\n";
    
    // ============================================
    // ALTERNATIVE: GAMIT ANG NATURAL LOG
    // ============================================
    cout << "=== ALTERNATIVE: NATURAL LOG ===\n\n";
    
    cout << "  ln(C) = " << log(C) << "\n";
    cout << "  ln(φ) = " << log(PHI) << "\n";
    cout << "  ln(C) / ln(φ) = " << log(C) / log(PHI) << "\n";
    cout << "  ln(C) - ln(φ) = " << log(C) - log(PHI) << "\n\n";
    
    cout << "  e^ln(C) = " << exp(log(C)) << "\n";
    cout << "  e^(ln(C)/φ) = " << exp(log(C) / PHI) << "\n\n";
    
    // ============================================
    // ANG PAG-ARAL NG LIMIT
    // ============================================
    cout << "=== LIMIT STUDY ===\n\n";
    cout << "  d | corr(d)×φ^d | diff mula sa previous\n";
    cout << "  --|--------------|----------------------\n";
    
    double prev = 0;
    for (int d = 0; d <= 20; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double scaled = corr * pow(PHI, d);
        double diff = scaled - prev;
        prev = scaled;
        
        cout << "  " << setw(2) << d << " | "
             << setw(12) << fixed << setprecision(6) << scaled << " | "
             << setw(8) << diff << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang diff ay bumababa sa φ-rate\n";
    cout << "  Ang limit ay maaaring may eksaktong φ-form\n\n";

    return 0;
}
