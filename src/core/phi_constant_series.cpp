// ============================================
// φ-CONSTANT SERIES
// Ang C bilang infinite series
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CONSTANT SERIES ===\n\n";
    
    // Ang C = corr(0) + Σ (corr(d) - corr(d-1)) × φ^d
    // 
    // Mas eksakto:
    // corr(d) = log_φ(1 + φ^(-d))
    //         = Σ_{k=1}^∞ (-1)^(k+1) × φ^(-dk) / (k × ln(φ))
    //
    // Sa φ-base:
    // corr(d) × φ^d = Σ_{k=1}^∞ (-1)^(k+1) × φ^(-d(k-1)) / (k × ln(φ))
    //
    // Para sa d → ∞:
    // C = 1 / ln(φ) = 1 / 0.481212 = 2.078...
    
    cout << "  Teoretikal na C = 1 / ln(φ)\n";
    cout << "  = 1 / " << LN_PHI << "\n";
    cout << "  = " << 1.0 / LN_PHI << "\n\n";
    
    cout << "  Experimental C ≈ 2.078\n";
    cout << "  Teoretikal C = " << 1.0 / LN_PHI << "\n";
    cout << "  Match: " << (abs(2.078 - 1.0/LN_PHI) < 0.01 ? "✅" : "❌") << "\n\n";
    
    // ============================================
    // ANG EKSAKTONG FORMULA
    // ============================================
    cout << "=== EKSAKTONG FORMULA ===\n\n";
    cout << "  corr(d) = log_φ(1 + φ^(-d))\n";
    cout << "  corr(d) × φ^d → 1/ln(φ) kapag d → ∞\n\n";
    
    cout << "  d | corr(d)×φ^d | 1/ln(φ) | Diff\n";
    cout << "  --|--------------|---------|------\n";
    
    for (int d = 5; d <= 20; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double scaled = corr * pow(PHI, d);
        double theoretical = 1.0 / LN_PHI;
        double diff = scaled - theoretical;
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(6) << scaled << " | "
             << setw(8) << theoretical << " | "
             << setw(9) << diff << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang constant C = 1/ln(φ) = " << 1.0/LN_PHI << "\n";
    cout << "  Ito ay eksaktong φ-based na halaga\n";
    cout << "  Ang correction ay may natural na φ-scaling\n\n";

    return 0;
}
