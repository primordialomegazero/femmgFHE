#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    double q = 4.0;
    double r = 0.527864;
    
    cout << "=== BRIDGE BUG ===\n\n";
    cout << "  q = " << q << "\n";
    cout << "  r = " << r << "\n";
    cout << "  LN_PHI = " << LN_PHI << "\n\n";
    
    double log_q = log(q) / LN_PHI;
    double log_one_plus = log(1.0 + r / (q * PHI)) / LN_PHI;
    double total = log_q + 1.0 + log_one_plus;
    
    cout << "  log(q) = " << log(q) << "\n";
    cout << "  log(q)/LN_PHI = " << log_q << "\n";
    cout << "  r/(q*PHI) = " << r / (q * PHI) << "\n";
    cout << "  1 + r/(q*PHI) = " << 1.0 + r / (q * PHI) << "\n";
    cout << "  log(1 + r/(q*PHI)) = " << log(1.0 + r / (q * PHI)) << "\n";
    cout << "  log(1 + r/(q*PHI))/LN_PHI = " << log_one_plus << "\n";
    cout << "  Total = " << total << "\n\n";
    
    cout << "  Expected: log_φ(7) = " << log(7.0) / LN_PHI << "\n";
    cout << "  Match: " << (abs(total - log(7.0)/LN_PHI) < 0.001 ? "YES" : "NO") << "\n";
    
    return 0;
}
