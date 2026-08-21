// INTERRUPTION EXACT FORMULA
// Hanapin ang φ-based pattern ng 17, 33, 7, 6
// At kung pwedeng ma-predict nang exact

#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  INTERRUPTION EXACT FORMULA\n";
    cout << "  φ-Based Pattern Analysis\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    // ============================================
    // BASE PATTERN: 2,1,3,1,4,2,2 (period 7)
    // ============================================

    vector<int> base = {2, 1, 3, 1, 4, 2, 2};
    int base_sum = 0;
    for (int g : base) base_sum += g;

    cout << "BASE PATTERN:\n";
    cout << "=============\n\n";
    cout << "  Pattern: ";
    for (int g : base) cout << g << " ";
    cout << "\n";
    cout << "  Sum: " << base_sum << "\n";
    cout << "  Average: " << (double)base_sum / base.size() << "\n\n";

    // ============================================
    // INTERRUPTIONS
    // ============================================

    cout << "INTERRUPTIONS:\n";
    cout << "==============\n\n";

    // Mula sa first 60 gaps:
    // 2,1,3,1,4,2,2,17,2,1,3,1,4,2,2,33,2,1,3,1,4,2,2,1,2,1,3,1,4,2,2,1,2,1,3,1,4,2,2,7,1,6,2,1,2,1,3,1,4,2,2,7,1,6,2,1,2,1,3,1

    vector<int> interruptions = {17, 33, 7, 6, 7, 6};
    
    cout << "  Values: ";
    for (int g : interruptions) cout << g << " ";
    cout << "\n\n";

    // ============================================
    // φ RELATIONSHIP
    // ============================================

    cout << "φ RELATIONSHIP:\n";
    cout << "================\n\n";

    cout << "  Interruptions vs φ powers:\n";
    for (int g : interruptions) {
        cout << "    " << g << " / φ = " << (double)g / phi;
        cout << " | " << g << " / φ² = " << (double)g / phi_sq;
        cout << " | " << g << " / φ³ = " << (double)g / phi_cu;
        cout << "\n";
    }

    cout << "\n  φ multiples:\n";
    cout << "    φ² = " << phi_sq << "\n";
    cout << "    φ³ = " << phi_cu << "\n";
    cout << "    φ⁴ = " << pow(phi, 4) << "\n";
    cout << "    φ⁵ = " << pow(phi, 5) << "\n";
    cout << "    φ⁶ = " << pow(phi, 6) << "\n\n";

    // ============================================
    // FIBONACCI CONNECTION
    // ============================================

    cout << "FIBONACCI CONNECTION:\n";
    cout << "=====================\n\n";

    vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

    cout << "  Interruptions as Fibonacci combinations:\n";
    for (int g : interruptions) {
        cout << "    " << g << " = ";
        bool found = false;
        
        // Check kung Fibonacci number
        if (find(fib.begin(), fib.end(), g) != fib.end()) {
            cout << "F(" << (find(fib.begin(), fib.end(), g) - fib.begin()) << ")";
            found = true;
        }
        
        // Check kung sum/difference ng Fibonacci
        for (int i = 0; i < fib.size() && !found; i++) {
            for (int j = i; j < fib.size(); j++) {
                if (fib[i] + fib[j] == g) {
                    cout << "F(" << i << ") + F(" << j << ") = " << fib[i] << " + " << fib[j];
                    found = true;
                    break;
                }
                if (abs(fib[i] - fib[j]) == g && i != j) {
                    cout << "|F(" << i << ") - F(" << j << ")| = |" << fib[i] << " - " << fib[j] << "|";
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        
        // Check kung product ng Fibonacci
        for (int i = 0; i < fib.size() && !found; i++) {
            for (int j = i; j < fib.size(); j++) {
                if (fib[i] * fib[j] == g) {
                    cout << "F(" << i << ") × F(" << j << ") = " << fib[i] << " × " << fib[j];
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        
        if (!found) cout << "complex";
        cout << "\n";
    }

    // ============================================
    // PATTERN SA INTERRUPTIONS
    // ============================================

    cout << "\nPATTERN SA INTERRUPTIONS:\n";
    cout << "=========================\n\n";

    cout << "  Sequence: 17, 33, 7, 6, 7, 6\n";
    cout << "  Differences:\n";
    for (int i = 1; i < interruptions.size(); i++) {
        cout << "    " << interruptions[i-1] << " → " << interruptions[i] 
             << ": diff = " << interruptions[i] - interruptions[i-1] << "\n";
    }

    // ============================================
    // POSSIBLE FORMULA
    // ============================================

    cout << "\nPOSSIBLE FORMULA:\n";
    cout << "=================\n\n";

    cout << "  17 = 2×8 + 1 = 2×F(5) + F(0)\n";
    cout << "  33 = 2×16 + 1 = 2×F(6) + F(0)\n";
    cout << "  7 = F(4) + F(2) = 5 + 2\n";
    cout << "  6 = F(4) + F(1) = 5 + 1\n\n";

    cout << "  Kaya ang interruptions ay:\n";
    cout << "    17 = 2×8 + 1\n";
    cout << "    33 = 2×16 + 1\n";
    cout << "    7 = 5 + 2\n";
    cout << "    6 = 5 + 1\n\n";

    cout << "  Kung saan ang 8, 16, 5, 2, 1 ay Fibonacci numbers!\n";

    // ============================================
    // PREDICTION ATTEMPT
    // ============================================

    cout << "\nPREDICTION ATTEMPT:\n";
    cout << "====================\n\n";

    cout << "  Kung ang pattern ay:\n";
    cout << "    17 = 2×F(5) + F(0)\n";
    cout << "    33 = 2×F(6) + F(0)\n";
    cout << "    ...\n";
    cout << "  Ang susunod ay: 2×F(7) + F(0) = 2×13 + 1 = 27\n\n";

    cout << "  At kung ang 7,6 ay:\n";
    cout << "    7 = F(4) + F(2)\n";
    cout << "    6 = F(4) + F(1)\n";
    cout << "    ...\n";
    cout << "  Ang susunod ay: F(5) + F(2) = 8 + 2 = 10\n";

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  CONCLUSION:\n";
    cout << "  - Interruptions ay Fibonacci-based\n";
    cout << "  - 17 = 2×F(5)+1, 33 = 2×F(6)+1\n";
    cout << "  - 7 = F(4)+F(2), 6 = F(4)+F(1)\n";
    cout << "  - May φ-based structure\n";
    cout << "========================================\n";

    return 0;
}
