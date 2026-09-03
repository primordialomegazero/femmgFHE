// ============================================
// φ-CORRECTION ENCODING
// I-encode ang correction term para sa
// exact conversion mula log papuntang normal
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
    cout << "  φ-CORRECTION ENCODING\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  x | log_φ(x)×φ | Correction | Recovered | Match?\n";
    cout << "  --|-------------|-----------|-----------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        double log_scaled = (log(x) / LN_PHI) * PHI;
        double correction = x - log_scaled;
        double recovered = log_scaled + correction;
        bool match = abs(recovered - x) < 0.001;
        
        cout << "  " << setw(3) << x << " | "
             << setw(11) << fixed << setprecision(4) << log_scaled << " | "
             << setw(9) << correction << " | "
             << setw(9) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  CORRECTION PATTERN\n";
    cout << "========================================\n\n";

    cout << "  x | Correction | φ^n? | F_n?\n";
    cout << "  --|------------|------|------\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 15; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        double log_scaled = (log(x) / LN_PHI) * PHI;
        double correction = x - log_scaled;
        
        // Hanapin kung may Fibonacci o φ^n pattern
        int nearest_fib = 0;
        double min_diff = 1e9;
        for (int i = 0; i < fib.size(); i++) {
            double diff = abs(correction - fib[i]);
            if (diff < min_diff) {
                min_diff = diff;
                nearest_fib = fib[i];
            }
        }
        
        cout << "  " << setw(3) << x << " | "
             << setw(10) << fixed << setprecision(4) << correction << " | "
             << setw(4) << nearest_fib << " | "
             << setw(4) << min_diff << "\n";
    }

    return 0;
}
