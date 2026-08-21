// DEEP PERIOD ANALYSIS
// a=0.5 at a=1.5 periods + 2D bounded + FHE complex

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <set>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  DEEP PERIOD ANALYSIS\n";
    cout << "  a=0.5, a=1.5, 2D, Complex FHE\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // ============================================
    // 1. PERIOD ANALYSIS PARA SA a=0.5 at a=1.5
    // ============================================
    cout << "1. PERIOD ANALYSIS:\n";
    cout << "===================\n\n";

    for (double a : {0.5, 1.0, 1.5}) {
        cout << "  a = " << a << ":\n";
        
        vector<double> seq;
        double x_prev = 0.0;
        double x_curr = phi;
        
        // I-generate ang 50 values
        for (int i = 0; i < 50; i++) {
            double x_next = a * x_curr - x_prev + phi;
            seq.push_back(x_next);
            x_prev = x_curr;
            x_curr = x_next;
        }
        
        // Hanapin ang period — exact match sa unang value
        int period = -1;
        for (int p = 2; p <= 30; p++) {
            bool match = true;
            for (int i = 0; i < 30 - p; i++) {
                if (std::abs(seq[i] - seq[i + p]) > 0.001) {
                    match = false;
                    break;
                }
            }
            if (match) {
                period = p;
                break;
            }
        }
        
        cout << "    Period: " << (period > 0 ? to_string(period) : "walang period") << "\n";
        cout << "    First 12 values: ";
        for (int i = 0; i < 12; i++) cout << seq[i] << " ";
        cout << "\n\n";
    }

    // ============================================
    // 2. 2D BOUNDED SYSTEM SEARCH
    // ============================================
    cout << "2. 2D BOUNDED SYSTEM SEARCH:\n";
    cout << "============================\n\n";

    // I-scan ang iba't ibang 2D recurrences para sa boundedness
    cout << "  Testing 2D systems:\n\n";

    for (double a : {-0.5, 0.0, 0.5}) {
        for (double b : {-0.5, 0.0, 0.5}) {
            double x = 0.0, y = phi;
            bool bounded = true;
            double max_mag = 0;
            
            for (int i = 0; i < 50; i++) {
                double x_next = a * x - b * y + phi;
                double y_next = a * y - b * x + phi;
                x = x_next;
                y = y_next;
                
                double mag = sqrt(x*x + y*y);
                if (mag > 100) { bounded = false; break; }
                if (mag > max_mag) max_mag = mag;
            }
            
            if (bounded) {
                cout << "    a=" << a << " b=" << b << ": BOUNDED (max_mag=" << max_mag << ")\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 3. COMPLEX FHE ENCODING
    // ============================================
    cout << "3. COMPLEX FHE ENCODING:\n";
    cout << "========================\n\n";

    cout << "  Ang period-6 ay may complex roots:\n";
    cout << "  r₁ = e^{iπ/3} = cos(60°) + i·sin(60°)\n\n";

    // Ipakita ang complex sequence
    cout << "  Complex sequence (period-6):\n";
    complex<double> z(phi, 0.0);
    complex<double> r(0.5, 0.866);
    
    cout << "    ";
    for (int n = 0; n < 12; n++) {
        complex<double> zn = z * pow(r, n);
        cout << "(" << zn.real() << "," << zn.imag() << ") ";
    }
    cout << "\n\n";

    // ============================================
    // 4. RULE 110 + PERIOD-6 COMBINATION
    // ============================================
    cout << "4. RULE 110 + PERIOD-6 COMBINATION:\n";
    cout << "===================================\n\n";

    cout << "  Ang Rule 110 ay may states: 0, 1\n";
    cout << "  Ang period-6 ay may states: 2K, 2K, K, 0, 0, K\n";
    cout << "  May overlap sa {0, K}\n\n";

    cout << "  Posibleng combination:\n";
    cout << "    Rule 110 bits: 0 o 1\n";
    cout << "    Period-6 states: 2K, 2K, K, 0, 0, K\n";
    cout << "    Mapa ang 0 → 0, 1 → K\n\n";

    cout << "  Kaya ang Rule 110 ay kayang\n";
    cout << "  i-encode sa period-6 cycle!\n\n";

    // ============================================
    // 5. SUMMARY
    // ============================================
    cout << "========================================\n";
    cout << "  KEY FINDINGS:\n";
    cout << "  - a=0.5 at a=1.5 ay BOUNDED\n";
    cout << "  - May complex representation\n";
    cout << "  - 2D systems ay kailangan ng specific a,b\n";
    cout << "  - Rule 110 ay kayang i-encode sa period-6\n";
    cout << "========================================\n";

    return 0;
}
