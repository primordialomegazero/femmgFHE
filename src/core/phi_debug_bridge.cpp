// ============================================
// φ-DEBUG BRIDGE
// I-debug ang transition computation
// ============================================

#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-DEBUG BRIDGE ===\n\n";
    
    double F = 7.0;
    double q = floor(F / PHI);
    double r = fmod(F, PHI);
    
    cout << "  F = " << F << "\n";
    cout << "  q = " << q << "\n";
    cout << "  r = " << r << "\n";
    cout << "  q×φ + r = " << q * PHI + r << "\n\n";
    
    double log_q = log(q) / LN_PHI;
    double log_F = log(F) / LN_PHI;
    double corr = log(1.0 + r / (q * PHI)) / LN_PHI;
    double total = log_q + 1.0 + corr;
    
    cout << "  log_φ(q) = " << log_q << "\n";
    cout << "  log_φ(F) = " << log_F << "\n";
    cout << "  correction = " << corr << "\n";
    cout << "  total = " << total << "\n";
    cout << "  Match: " << (abs(total - log_F) < 0.001 ? "✅" : "❌") << "\n\n";
    
    // Test para sa iba pang values
    cout << "  Test para sa iba pang values:\n\n";
    cout << "  F | q | r | log_φ(F) exact | bridge | Diff\n";
    cout << "  --|---|---|----------------|--------|------\n";
    
    for (double F_test : {2.0, 3.0, 5.0, 7.0, 8.0, 13.0, 21.0}) {
        double q_test = floor(F_test / PHI);
        double r_test = fmod(F_test, PHI);
        double log_F_test = log(F_test) / LN_PHI;
        double log_q_test = log(q_test) / LN_PHI;
        double corr_test = log(1.0 + r_test / (q_test * PHI)) / LN_PHI;
        double bridge_test = log_q_test + 1.0 + corr_test;
        
        cout << "  " << setw(2) << F_test << " | "
             << setw(2) << q_test << " | "
             << setw(4) << fixed << setprecision(2) << r_test << " | "
             << setw(10) << log_F_test << " | "
             << setw(8) << bridge_test << " | "
             << setw(6) << (log_F_test - bridge_test) << "\n";
    }

    return 0;
}
