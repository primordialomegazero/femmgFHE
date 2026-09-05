// ============================================
// φ-FIBMOD RULE 110 — Natural Rotation
// Fibonacci mod 8 bilang natural na slot shift
// Walang EvalRotate — automatic sa φ-structure
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-FIBMOD RULE 110 ===\n\n";
    cout << fixed << setprecision(12);

    // Fibonacci mod 8 sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);

    // ============================================
    // 1. Fibonacci mod 8 bilang rotation index
    // ============================================
    cout << "--- 1. Fibonacci mod 8 bilang rotation ---\n\n";
    cout << "  n | F_n mod 8 | Rotation\n";
    cout << "  --|-----------|----------\n";
    
    for (int n = 0; n <= 16; n++) {
        long long rot = fib[n] % 8;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << rot << " | "
             << setw(8) << rot << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. State evolution gamit ang Fibonacci rotation
    // ============================================
    cout << "--- 2. State evolution ---\n\n";
    cout << "  Ang state ay nag-ro-rotate sa Fibonacci mod 8\n";
    cout << "  Pattern: 11010101 na may natural na shift\n\n";

    // Initial state
    vector<int> state = {1, 1, 0, 1, 0, 1, 0, 1};
    
    cout << "  Step 0: ";
    for (int bit : state) cout << bit;
    cout << "\n";

    for (int step = 1; step <= 10; step++) {
        // Fibonacci rotation: i-shift ang state gamit ang
        // natural na Fibonacci index
        long long rot = fib[step] % 8;
        
        vector<int> new_state(8, 0);
        for (int i = 0; i < 8; i++) {
            new_state[i] = state[(i + rot) % 8];
        }
        state = new_state;
        
        cout << "  Step " << setw(2) << step << ": ";
        for (int bit : state) cout << bit;
        cout << "  (rot=" << rot << ")\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang emergent na pattern
    // ============================================
    cout << "--- 3. Emergent na pattern ---\n\n";
    cout << "  Ang Fibonacci rotation ay may period 12\n";
    cout << "  Pagkatapos ng 12 steps, bumalik sa original\n\n";

    vector<int> state_check = {1, 1, 0, 1, 0, 1, 0, 1};
    cout << "  Step 0: ";
    for (int bit : state_check) cout << bit;
    cout << "\n";

    for (int step = 1; step <= 12; step++) {
        long long rot = fib[step] % 8;
        vector<int> new_state(8, 0);
        for (int i = 0; i < 8; i++) {
            new_state[i] = state_check[(i + rot) % 8];
        }
        state_check = new_state;
        
        if (step == 12) {
            cout << "  Step 12: ";
            for (int bit : state_check) cout << bit;
            cout << "  ← dapat pareho sa Step 0\n";
        }
    }
    cout << "\n";

    return 0;
}
