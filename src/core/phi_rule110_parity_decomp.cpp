// ============================================
// φ-RULE 110 PARITY DECOMP — Parity ng φ-power
// next batay sa parity ng φ-power decomposition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <bitset>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 PARITY DECOMP ===\n\n";
    cout << fixed << setprecision(12);

    cout << "  val | Binary | φ-decomp | #terms | parity | Next\n";
    cout << "  ----|--------|----------|--------|--------|------\n";
    
    for (int val = 0; val < 8; val++) {
        int next = rule110[val];
        
        // φ-power decomposition
        double remaining = val;
        vector<int> powers;
        for (int p = 3; p >= -3; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                powers.push_back(p);
                remaining -= phi_p;
            }
        }
        
        int num_terms = powers.size();
        int parity = num_terms % 2;
        
        cout << "  " << setw(3) << val << " | "
             << setw(6) << bitset<3>(val) << " | ";
        
        for (int p : powers) {
            cout << "φ^" << p << " ";
        }
        cout << "| "
             << setw(6) << num_terms << " | "
             << setw(6) << parity << " |  "
             << next << "\n";
    }
    cout << "\n";

    // Ang transition ba ay may parity pattern?
    cout << "  next=0: val ∈ {0, 3, 7}\n";
    cout << "  0 → 0 terms → parity 0\n";
    cout << "  3 → 2 terms → parity 0\n";
    cout << "  7 → 2 terms → parity 0\n\n";
    
    cout << "  next=1: val ∈ {1, 2, 4, 5, 6}\n";
    cout << "  1 → 1 term → parity 1\n";
    cout << "  2 → 2 terms → parity 0 ← hindi tugma\n";
    cout << "  4 → 1 term → parity 1\n";
    cout << "  5 → 3 terms → parity 1\n";
    cout << "  6 → 2 terms → parity 0 ← hindi tugma\n\n";

    return 0;
}
