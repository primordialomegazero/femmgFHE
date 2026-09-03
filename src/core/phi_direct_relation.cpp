// ============================================
// φ-DIRECT RELATION
// Hanapin kung may direct relation sa pagitan
// ng normal addition at log multiplication
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
    cout << "  φ-DIRECT RELATION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  x | y | x+y | log_φ(x×y)×φ | Diff | Ratio\n";
    cout << "  --|---|-----|--------------|------|-------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double y : {2.0, 3.0, 5.0, 8.0, 13.0}) {
            double normal = x + y;
            double log_scaled = (log(x * y) / LN_PHI) * PHI;
            double diff = normal - log_scaled;
            double ratio = normal / log_scaled;
            
            cout << "  " << setw(2) << x << " | "
                 << setw(2) << y << " | "
                 << setw(3) << normal << " | "
                 << setw(12) << fixed << setprecision(4) << log_scaled << " | "
                 << setw(5) << diff << " | "
                 << setw(5) << ratio << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  PATTERN SEARCH\n";
    cout << "========================================\n\n";

    cout << "  Hanapin kung may formula:\n";
    cout << "  x + y = k × log_φ(x × y) × φ\n\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double y : {2.0, 3.0, 5.0, 8.0, 13.0}) {
            double normal = x + y;
            double log_scaled = (log(x * y) / LN_PHI) * PHI;
            double k = normal / log_scaled;
            
            if (abs(k - 1.0) < 0.1) {
                cout << "  x=" << x << ", y=" << y << ": k = " 
                     << fixed << setprecision(4) << k << "\n";
            }
        }
    }

    return 0;
}
