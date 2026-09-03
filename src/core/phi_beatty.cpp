// ============================================
// φ-BEATTY
// Beatty sequences bilang natural indexing
// floor(nφ) at floor(nφ²) partition integers
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double PHI2 = PHI * PHI;
    
    cout << "=== φ-BEATTY ===\n\n";
    cout << "  Beatty theorem: floor(nφ) at floor(nφ²)\n";
    cout << "  ay nagpa-partition ng positive integers\n";
    cout << "  1/φ + 1/φ² = " << 1.0/PHI + 1.0/PHI2 << "\n\n";
    
    cout << "  n | floor(nφ) | floor(nφ²) | Match?\n";
    cout << "  --|-----------|------------|-------\n";
    
    vector<bool> used(100, false);
    bool ok = true;
    
    for (int n = 1; n <= 15; n++) {
        int beatty_phi = (int)floor(n * PHI);
        int beatty_phi2 = (int)floor(n * PHI2);
        
        bool phi_used = used[beatty_phi];
        bool phi2_used = used[beatty_phi2];
        used[beatty_phi] = true;
        used[beatty_phi2] = true;
        
        bool match = !phi_used && !phi2_used;
        if (!match) ok = false;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << beatty_phi << " | "
             << setw(10) << beatty_phi2 << " | "
             << (match ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  Lahat unique? " << (ok ? "✅" : "❌") << "\n\n";
    
    // ============================================
    // BEATTY + LOG SPACE
    // ============================================
    cout << "=== BEATTY + LOG SPACE ===\n\n";
    cout << "  Ang Beatty index ay may natural na φ-based na log\n";
    cout << "  log_φ(floor(nφ)) ≈ log_φ(n) + 1\n";
    cout << "  log_φ(floor(nφ²)) ≈ log_φ(n) + 2\n\n";
    
    cout << "  n | floor(nφ) | log_φ(floor(nφ)) | log_φ(n)+1 | Diff\n";
    cout << "  --|-----------|-------------------|------------|------\n";
    
    for (int n = 1; n <= 15; n++) {
        double beatty = floor(n * PHI);
        double log_beatty = log(beatty) / log(PHI);
        double log_n_plus_1 = log(n) / log(PHI) + 1.0;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(0) << beatty << " | "
             << setw(13) << log_beatty << " | "
             << setw(12) << log_n_plus_1 << " | "
             << setw(6) << (log_beatty - log_n_plus_1) << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang Beatty indexing ay nagbibigay ng natural na\n";
    cout << "  partition na maaaring gamitin para sa sync\n\n";

    return 0;
}
