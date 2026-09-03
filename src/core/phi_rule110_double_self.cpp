// ============================================
// φ-RULE 110 DOUBLE SELF-REFERENTIAL
//
// next = f(current, previous)
// May memory ng previous state!
//
// φ back-and-forth:
// φ^n + φ^(n+1) = φ^(n+2) (forward)
// φ^(n+2) - φ^(n+1) = φ^n (backward)
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 DOUBLE SELF-REF\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: FORWARD AND BACKWARD
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: FORWARD + BACKWARD\n";
    cout << "========================================\n\n";

    int N = 16;
    vector<int> state(N, 0);
    state[7] = 1;
    state[8] = 1;

    vector<int> prev_state = state;

    cout << "  Gen | Forward | Backward | Both Match?\n";
    cout << "  ----|---------|----------|------------\n";

    for (int gen = 0; gen < 10; gen++) {
        // FORWARD
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = state[(i + N - 1) % N];
            int C = state[i];
            int R = state[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        
        // BACKWARD (reverse check)
        bool both_match = (next == prev_state);
        
        cout << "  " << setw(3) << gen << " | ";
        for (int i = 0; i < 8; i++) cout << next[i];
        cout << " | ";
        for (int i = 0; i < 8; i++) cout << prev_state[i];
        cout << " | " << (both_match ? "✅" : "❌") << "\n";
        
        prev_state = state;
        state = next;
    }

    cout << "\n";

    // ============================================
    // TEST 2: φ BACK-AND-FORTH
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: φ BACK-AND-FORTH\n";
    cout << "========================================\n\n";

    cout << "  Forward (Fibonacci): 1, 1, 2, 3, 5, 8, 13, 21\n";
    cout << "  Backward (reverse): 21, 13, 8, 5, 3, 2, 1, 1\n\n";

    vector<long long> fib = {1, 1, 2, 3, 5, 8, 13, 21};
    
    cout << "  n | Forward F(n) | Backward F(8-n) | φ-Relation?\n";
    cout << "  --|-------------|----------------|------------\n";

    for (int n = 0; n < 8; n++) {
        double forward = fib[n];
        double backward = fib[7 - n];
        double phi_rel = forward / backward;
        
        cout << "  " << n << " | "
             << setw(11) << forward << " | "
             << setw(14) << backward << " | "
             << setw(10) << fixed << setprecision(3) << phi_rel << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 3: DOUBLE SELF-REF EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: DOUBLE SELF-REF EVOLUTION\n";
    cout << "========================================\n\n";

    // Simulate: next = f(current, previous)
    // next[i] = rule110[(L×4 + C×2 + R)] kung saan
    // C = current[i], L = current[i-1], R = current[i+1]
    // At previous ay ginagamit para sa correction

    vector<int> cur(N, 0);
    cur[7] = 1;
    cur[8] = 1;
    vector<int> prev = cur;

    cout << "  Gen | State | Density\n";
    cout << "  ----|----------------|--------\n";

    for (int gen = 0; gen < 20; gen++) {
        int ones = 0;
        for (int bit : cur) ones += bit;
        
        cout << "  " << setw(3) << gen << " | ";
        for (int i = 0; i < N; i++) cout << cur[i];
        cout << " | " << ones << "/" << N << "\n";
        
        // DOUBLE SELF-REF: 
        // next = rule110(current) XOR (current != previous)
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = cur[(i + N - 1) % N];
            int C = cur[i];
            int R = cur[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            
            int output = rule110[pattern];
            
            // SELF-REF CORRECTION:
            // Kung ang previous ay iba sa current, i-adjust
            int self_correction = (cur[i] != prev[i]) ? 1 : 0;
            next[i] = output ^ self_correction;  // XOR!
        }
        
        prev = cur;
        cur = next;
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  DOUBLE SELF-REF COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Forward + backward analysis\n";
    cout << "  ✅ φ back-and-forth: Fibonacci\n";
    cout << "  ✅ Double self-ref: next = f(cur, prev)\n";
    cout << "  ✅ May memory ng previous state\n\n";

    return 0;
}
