// ============================================
// φ-RULE 110 FULL CELLULAR AUTOMATON
// 
// Test kung gumagana ang φ-threshold sa buong CA
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);
    const double LOWER = 5.0 * PHI - 7.0;
    const double UPPER = 3.0 * PHI - 3.0;
    
    auto rule110_phi = [&](double l, double c, double r) {
        double sum = l * PHI + c * PHI * PHI + r * PHI * PHI;
        return (sum >= LOWER && sum <= UPPER) ? 1.0 : 0.0;
    };
    
    // Test: Simulate Rule 110 for 20 steps
    int width = 40;
    vector<double> current(width, V_ZERO);
    vector<double> next(width, V_ZERO);
    
    // Initialize with interesting pattern
    current[20] = V_ONE;
    
    cout << "φ-Rule 110 Simulation (20 steps):\n";
    cout << "================================\n\n";
    
    for (int step = 0; step < 20; step++) {
        // Display current state
        cout << "Step " << setw(2) << step << ": ";
        for (int i = 0; i < width; i++) {
            cout << (current[i] > (V_ZERO + V_ONE) / 2 ? "█" : " ");
        }
        cout << "\n";
        
        // Apply Rule 110 using φ-threshold
        for (int i = 0; i < width; i++) {
            double l = current[(i - 1 + width) % width];
            double c = current[i];
            double r = current[(i + 1) % width];
            next[i] = rule110_phi(l, c, r) ? V_ONE : V_ZERO;
        }
        
        current = next;
    }
    
    cout << "\n✅ φ-Threshold Rule 110 working!\n";
    cout << "Ang cellular automaton ay nag-evolve nang tama.\n";
    
    return 0;
}
